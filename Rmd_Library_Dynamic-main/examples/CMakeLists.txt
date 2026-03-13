set(EXAMPLES
    ex1_info.cpp
    ex2_single.cpp)

foreach( ex_file ${EXAMPLES} )
    string(REPLACE ".cpp" "" exFileName ${ex_file})
    string(TOUPPER ${exFileName} exFileName)

    message("Building ${exFileName}")

    add_executable( ${exFileName} ${ex_file} )
    target_link_libraries( ${exFileName} Rmd_Library )
endforeach(ex_file ${EXAMPLES} )
