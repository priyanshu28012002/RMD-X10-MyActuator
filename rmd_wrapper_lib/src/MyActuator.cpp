#include "MyActuator.hpp"
#include "x10_api.hpp"

#include <chrono>
#include <cstring>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <sys/mman.h>

static uint64_t epochMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

MyActuator::MyActuator(int motorCount, const std::string &port)
    : motorCount_(motorCount), port_(port), driver_(std::make_unique<X10ApiSerial>(port)), motorStatus_(motorCount), pollRoundRobin_(0)
{
    if (motorCount < 1 || motorCount > 32)
        throw std::invalid_argument("motorCount must be 1..32");

    for (int i = 0; i < motorCount_; ++i)
    {
        motorStatus_[i].id = static_cast<uint8_t>(i + 1);
    }
    speeds.resize(motorCount);
    for (int i = 0; i < motorCount; i++)
    {
        speeds[i] = 0;
    }

    driver_->rmdX10_init();

    commandExecuter = std::thread(&MyActuator::executerLoop, this);
}

MyActuator::~MyActuator()
{
    stop();
    driver_->rmdX10_shut_down();
}

void MyActuator::start()
{
    if (running_.load())
        return;

    stopRequested_.store(false);
    running_.store(true);
    rtThread_ = std::thread(&MyActuator::rtLoop, this);
}

void MyActuator::stop()
{
    if (!running_.load())
        return;

    stopRequested_.store(true);
    if (rtThread_.joinable())
        rtThread_.join();

    running_.store(false);
}

bool MyActuator::isRunning() const
{
    return running_.load();
}

void MyActuator::executerLoop()
{

    int lastSpeed = 0;
    while (true)
    {

       
            auto result = driver_->speedControl(1, speeds[0]);
            if (result == 1)
                lastSpeed = speeds[0];

            // if(result)

            std::cout << "Result -> " << (int)result << std::endl;
       

        // std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
void MyActuator::rtLoop()
{
#if defined(__linux__)
    struct sched_param sp{};
    sp.sched_priority = sched_get_priority_max(SCHED_FIFO) - 1;
    if (::pthread_setschedparam(::pthread_self(), SCHED_FIFO, &sp) != 0)
        std::cerr << "[MyActuator] Warning: could not set SCHED_FIFO (run as root?)\n";

    if (::mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
        std::cerr << "[MyActuator] Warning: mlockall failed\n";
#endif

    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::microseconds;
    const Duration period{LOOP_US};

    auto nextWake = Clock::now() + period;

    while (!stopRequested_.load())
    {
        const auto cycleStart = Clock::now();

        // ── 1. Emergency / queued commands ───────────────────────────────────
        dispatchPendingCommands();

        // ── 2. Poll motor states (round-robin across motors × 3 state cmds) ──
        pollMotorStates();

        // ── 3. Fire status callback ──────────────────────────────────────────
        {
            std::lock_guard<std::mutex> cbLk(cbMutex_);
            if (statusCb_)
            {
                std::vector<MotorStatus> snap;
                {
                    std::lock_guard<std::mutex> sLk(statusMutex_);
                    snap = motorStatus_;
                }
                statusCb_(snap);
            }
        }

        // ── 4. Measure overrun ───────────────────────────────────────────────
        const auto elapsed = std::chrono::duration_cast<Duration>(Clock::now() - cycleStart);
        const int64_t slack = static_cast<int64_t>(LOOP_US) - elapsed.count();
        if (slack < 0)
        {
            int64_t overrun = -slack;
            int64_t prev = worstOverrunUs_.load();
            while (overrun > prev && !worstOverrunUs_.compare_exchange_weak(prev, overrun))
                ;
        }

        ++cycleCount_;

        // ── 5. Sleep until next 20 ms boundary ───────────────────────────────
        std::this_thread::sleep_until(nextWake);
        nextWake += period;
    }
}

void MyActuator::dispatchPendingCommands()
{
    std::unique_lock<std::mutex> lk(cmdMutex_);

    int dispatched = 0;
    while (!cmdQueue_.empty() && dispatched < MAX_CMD_PER_CYCLE)
    {
        PrioritisedCommand pc = cmdQueue_.top();
        cmdQueue_.pop();
        lk.unlock();

        executeCommand(pc.cmd);
        ++dispatched;

        lk.lock();
    }
}

void MyActuator::executeCommand(const MotorCommand &cmd)
{
    switch (cmd.type)
    {
    // ── Emergency: stop everything ──────────────────────────────────────────
    case CommandType::EMERGENCY_STOP_ALL:
        driver_->Motor_stop(0XCD);
        break;
    // ── Motion ──────────────────────────────────────────────────────────────
    case CommandType::SET_SPEED:
        driver_->speedControl(cmd.motorId, cmd.param1);
        break;

    case CommandType::SET_TORQUE:
        driver_->torqueControl(cmd.motorId, cmd.param1);
        break;
    case CommandType::SET_INC_POSITION:
        driver_->increment_control(cmd.motorId, static_cast<uint16_t>(cmd.param2), cmd.param1);
        break;

    // ── Power ───────────────────────────────────────────────────────────────
    case CommandType::MOTOR_SHUTDOWN:
        driver_->Motor_shut_down(0XCD);
        break;

    case CommandType::MOTOR_RESET:
        driver_->Motor_reset(0XCD);
        break;

    // ── Explicit state reads (injected by pollMotorStates) ──────────────────
    case CommandType::READ_STATE1:
    {
        int16_t data[4] = {};
        if (driver_->Motor_state1(cmd.motorId, data) == 0)
        {
            std::lock_guard<std::mutex> lk(statusMutex_);
            auto &s = statusRef(cmd.motorId);
            s.temperature = static_cast<int8_t>(data[0]);
            s.voltage = data[1];
            s.errorState = static_cast<uint8_t>(data[2]);
            s.online = true;
            s.lastUpdated = nowMs();

            // Fault callback
            if (s.errorState)
            {
                std::lock_guard<std::mutex> cbLk(cbMutex_);
                if (faultCb_)
                    faultCb_(cmd.motorId, s.errorState);
            }
        }
        break;
    }

    case CommandType::READ_STATE2:
    {
        int16_t data[4] = {};
        if (driver_->Motor_state2(cmd.motorId, data) == 0)
        {
            std::lock_guard<std::mutex> lk(statusMutex_);
            auto &s = statusRef(cmd.motorId);
            s.temperature2 = static_cast<int8_t>(data[0]);
            s.torqueCurrent = data[1];
            s.speed = data[2];
            s.encoder = data[3];
            s.online = true;
            s.lastUpdated = nowMs();
        }
        break;
    }

    case CommandType::READ_STATE3:
    {
        int16_t data[4] = {};
        if (driver_->Motor_state3(cmd.motorId, data) == 0)
        {
            std::lock_guard<std::mutex> lk(statusMutex_);
            auto &s = statusRef(cmd.motorId);
            s.phaseA = data[0];
            s.phaseB = data[1];
            s.phaseC = data[2];
            s.online = true;
            s.lastUpdated = nowMs();
        }
        break;
    }

    default:
        break;
    }
}

void MyActuator::pollMotorStates()
{
    const int total = motorCount_ * 3;
    const uint8_t slot = pollRoundRobin_ % total;

    uint8_t motorIdx = slot / 3;
    uint8_t stateIdx = slot % 3;

    uint8_t motorId = static_cast<uint8_t>(motorIdx + 1);
    CommandType ct = (stateIdx == 0)   ? CommandType::READ_STATE1
                     : (stateIdx == 1) ? CommandType::READ_STATE2
                                       : CommandType::READ_STATE3;

    MotorCommand cmd{};
    cmd.type = ct;
    cmd.motorId = motorId;
    executeCommand(cmd);

    pollRoundRobin_ = static_cast<uint8_t>((pollRoundRobin_ + 1) % total);
}

void MyActuator::emergencyStopAll()
{
    PrioritisedCommand pc;
    pc.emergency = true;
    pc.cmd.type = CommandType::EMERGENCY_STOP_ALL;
    pc.cmd.emergency = true;

    std::lock_guard<std::mutex> lk(cmdMutex_);
    cmdQueue_.push(pc);
}

void MyActuator::emergencyStop(uint8_t motorId)
{
    if (!validId(motorId))
        return;

    PrioritisedCommand pc;
    pc.emergency = true;
    pc.cmd.type = CommandType::EMERGENCY_STOP_ONE;
    pc.cmd.motorId = motorId;
    pc.cmd.emergency = true;

    std::lock_guard<std::mutex> lk(cmdMutex_);
    cmdQueue_.push(pc);
}

void MyActuator::setSpeed(uint8_t motorId, int32_t speedDps)
{
    // if (!validId(motorId)) return;

    speeds[motorId] = speedDps;
    std::cout << (int)motorId << " sssssssssssss " << speeds[motorId] << std::endl;

    // PrioritisedCommand pc;
    // pc.emergency   = false;
    // pc.cmd.type    = CommandType::SET_SPEED;
    // pc.cmd.motorId = motorId;
    // pc.cmd.param1  = speedDps;

    // std::lock_guard<std::mutex> lk(cmdMutex_);
    // cmdQueue_.push(pc);
}

void MyActuator::setSpeedAll(int32_t speedDps)
{
    for (int i = 0; i < 2; i++)
    {
        speeds[i] = (int)speedDps;
    }

    // for (auto x : speeds)
    // {
    // std::cout << (int)x <<" ";
    // }
    // std::cout <<std::endl;

    // PrioritisedCommand pc;
    // pc.emergency   = false;
    // pc.cmd.type    = CommandType::SET_SPEED;
    // pc.cmd.motorId = motorId;
    // pc.cmd.param1  = speedDps;

    // std::lock_guard<std::mutex> lk(cmdMutex_);
    // cmdQueue_.push(pc);
}

void MyActuator::setTorque(uint8_t motorId, int32_t iqControl)
{
    if (!validId(motorId))
        return;

    PrioritisedCommand pc;
    pc.emergency = false;
    pc.cmd.type = CommandType::SET_TORQUE;
    pc.cmd.motorId = motorId;
    pc.cmd.param1 = iqControl;

    std::lock_guard<std::mutex> lk(cmdMutex_);
    cmdQueue_.push(pc);
}

void MyActuator::setAbsolutePosition(uint8_t motorId, int32_t angle, uint16_t maxSpeed)
{
    if (!validId(motorId))
        return;

    PrioritisedCommand pc;
    pc.emergency = false;
    pc.cmd.type = CommandType::SET_ABS_POSITION;
    pc.cmd.motorId = motorId;
    pc.cmd.param1 = angle;
    pc.cmd.param2 = static_cast<int32_t>(maxSpeed);

    std::lock_guard<std::mutex> lk(cmdMutex_);
    cmdQueue_.push(pc);
}

void MyActuator::setIncrementalPosition(uint8_t motorId, int32_t angleDelta, uint16_t maxSpeed)
{
    if (!validId(motorId))
        return;

    PrioritisedCommand pc;
    pc.emergency = false;
    pc.cmd.type = CommandType::SET_INC_POSITION;
    pc.cmd.motorId = motorId;
    pc.cmd.param1 = angleDelta;
    pc.cmd.param2 = static_cast<int32_t>(maxSpeed);

    std::lock_guard<std::mutex> lk(cmdMutex_);
    cmdQueue_.push(pc);
}

void MyActuator::shutdownMotor(uint8_t motorId)
{
    if (!validId(motorId))
        return;
    PrioritisedCommand pc;
    pc.cmd.type = CommandType::MOTOR_SHUTDOWN;
    pc.cmd.motorId = motorId;
    std::lock_guard<std::mutex> lk(cmdMutex_);
    cmdQueue_.push(pc);
}

void MyActuator::resetMotor(uint8_t motorId)
{
    if (!validId(motorId))
        return;
    PrioritisedCommand pc;
    pc.cmd.type = CommandType::MOTOR_RESET;
    pc.cmd.motorId = motorId;
    std::lock_guard<std::mutex> lk(cmdMutex_);
    cmdQueue_.push(pc);
}

MotorStatus MyActuator::getStatus(uint8_t motorId) const
{
    if (!validId(motorId))
        return {};

    std::lock_guard<std::mutex> lk(statusMutex_);
    return motorStatus_[motorId - 1];
}

std::vector<MotorStatus> MyActuator::getAllStatus() const
{
    std::lock_guard<std::mutex> lk(statusMutex_);
    return motorStatus_;
}

bool MyActuator::isMotorOnline(uint8_t motorId, uint32_t timeoutMs) const
{
    if (!validId(motorId))
        return false;

    std::lock_guard<std::mutex> lk(statusMutex_);
    const auto &s = motorStatus_[motorId - 1];
    return s.online && (nowMs() - s.lastUpdated < timeoutMs);
}

void MyActuator::setStatusCallback(StatusCallback cb)
{
    std::lock_guard<std::mutex> lk(cbMutex_);
    statusCb_ = std::move(cb);
}

void MyActuator::setFaultCallback(FaultCallback cb)
{
    std::lock_guard<std::mutex> lk(cbMutex_);
    faultCb_ = std::move(cb);
}

uint64_t MyActuator::cycleCount() const { return cycleCount_.load(); }
int64_t MyActuator::worstOverrunUs() const { return worstOverrunUs_.load(); }

MotorStatus &MyActuator::statusRef(uint8_t id)
{
    return motorStatus_[id - 1];
}

const MotorStatus &MyActuator::statusRef(uint8_t id) const
{
    return motorStatus_[id - 1];
}

bool MyActuator::validId(uint8_t id) const
{
    return id >= 1 && id <= static_cast<uint8_t>(motorCount_);
}

uint64_t MyActuator::nowMs() const
{
    return epochMs();
}