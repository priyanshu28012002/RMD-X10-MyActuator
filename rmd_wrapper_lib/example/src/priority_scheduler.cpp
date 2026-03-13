#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>

class Task
{
public:
    int id;
    int priority;
    int x;
    int y;
    std::function<int(int, int)> func;

    Task(int id_, int priority_, int x_, int y_, std::function<int(int, int)> f)
        : id(id_), priority(priority_), x(x_), y(y_), func(f) {}
};

struct ComparePriority
{
    bool operator()(const Task &a, const Task &b)
    {
        return a.priority < b.priority;
    }
};

class PriorityScheduler
{
private:
    std::priority_queue<Task, std::vector<Task>, ComparePriority> task_queue;
    std::mutex mtx;

    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_int_distribution<> priority_dist{0, 5};
    std::uniform_int_distribution<> value_dist{1, 100};

public:
    void addTask(int id, int priority, int x, int y, std::function<int(int, int)> func)
    {
        std::lock_guard<std::mutex> lock(mtx);

        task_queue.emplace(id, priority, x, y, func);

        std::cout << "Task Added ID " << id
                  << " Priority " << priority
                  << " X " << x
                  << " Y " << y << std::endl;
    }

    void produce()
    {
        int taskCount = 10;

        for (int i = 0; i < taskCount; i)
        {
            int p = priority_dist(gen);
            int x = value_dist(gen);
            int y = value_dist(gen);

            addTask(i, p, x, y, Multiply);

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void run()
    {
        while (true)
        {

            if (!task_queue.empty())
            {
                std::lock_guard<std::mutex> lock(mtx);

                Task t = task_queue.top();
                task_queue.pop();

                int result = t.func(t.x, t.y);

                std::cout << "Running Task " << t.id
                          << " Priority " << t.priority
                          << " Result " << result << std::endl;
            }
            else
            {
                std::cout << "No Task\n";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    static int Multiply(int a, int b)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return a * b;
    }

    static int Add(int a, int b)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return a + b;
    }

    static int Divide(int a, int b)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return a / b;
    }

    static int Sub(int a, int b)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return a - b;
    }
};

int main()
{
    PriorityScheduler scheduler;

    std::thread producer(&PriorityScheduler::produce, &scheduler);

    std::thread consumer(&PriorityScheduler::run, &scheduler);

    producer.join();
    consumer.join();
}