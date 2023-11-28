include(CMakeParseArguments)

function(add_quickjs_library csource)
    set(options MODULE ENTRY)
    set(singleValueArgs STATIC)
    set(multiValueArgs SOURCES IMPORTS FLAGS)
    cmake_parse_arguments(QJS "${options}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    set(coutput "${CMAKE_CURRENT_BINARY_DIR}/${csource}")

    set(gen_args -c)

    set(jsdeps_args "")
    if(QJS_IMPORTS)
        foreach(jsdep ${QJS_IMPORTS})
            list(APPEND jsdeps_args -M ${jsdep})
        endforeach()
    endif()

    set(flags_args "")
    if(QJS_FLAGS)
        foreach(flag ${QJS_FLAGS})
            list(APPEND flags_args "${flag}")
        endforeach()
    endif()

    set(module_arg "")
    if(QJS_MODULE)
        set(module_arg -m)
    endif()

    if(QJS_STATIC)
        set(gen_args -s ${QJS_STATIC})
    endif()
    
    if(QJS_ENTRY)
        set(gen_args -e)
    endif()

    add_custom_command(
        OUTPUT ${coutput}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND qjsc ${gen_args} -o ${coutput} ${jsdeps_args} ${module_arg} ${QJS_SOURCES} ${flags_args}
        DEPENDS ${jsfiles}
    )
endfunction()