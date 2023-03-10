
set(DEBUG_EMBER_MODULE OFF)





function(ember_bake)
    set(options)
    set(oneValueArgs SYMBOL FILE)
    set(multiValueArgs)
    cmake_parse_arguments(EMBER_BAKE "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})


    # parse target variable

    list(LENGTH EMBER_BAKE_UNPARSED_ARGUMENTS EMBER_BAKE_UNPARSED_LENGTH)

    if (${EMBER_BAKE_UNPARSED_LENGTH} LESS 1)
        message(FATAL_ERROR "No target specified for ember_bake")
    elseif(${EMBER_BAKE_UNPARSED_LENGTH} GREATER 1)
        list(SUBLIST EMBER_BAKE_UNPARSED_ARGUMENTS 1 ${EMBER_BAKE_UNPARSED_LENGTH}-1 EMBER_BAKE_UNPARSED_EXTRA)
        message(FATAL_ERROR "Unexpected arguments provided to call of ember_bake : ${EMBER_BAKE_UNPARSED_EXTRA}")
    endif()

    list(GET EMBER_BAKE_UNPARSED_ARGUMENTS 0 EMBER_BAKE_TARGET)

    # if (NOT DEFINED ${EMBER_BAKE_SYMBOL})
    #     message(FATAL_ERROR "SYMBOL argument not provided in call to ember_bake.")
    # endif()

    if (NOT DEFINED EMBER_BAKE_FILE)
        message(FATAL_ERROR "FILE argument not provided in call to ember_bake.")
    endif()

    set(GENERATED_HEADER "${EMBER_BAKE_FILE}.hpp")
    set(GENERATED_SOURCE "${EMBER_BAKE_FILE}.cpp")

    if(${DEBUG_EMBER_MODULE})
        message("GENERATED_HEADER : ${GENERATED_HEADER}")
        message("GENERATED_SOURCE : ${GENERATED_SOURCE}")
    endif()

    get_filename_component(EMBER_BAKE_WORK_DIR ${EMBER_BAKE_FILE} DIRECTORY)

    add_custom_command(
            OUTPUT "${GENERATED_HEADER}" "${GENERATED_SOURCE}"
            COMMAND $<TARGET_FILE:ember> -i "${EMBER_BAKE_FILE}"
            DEPENDS ${EMBER_BAKE_FILE}
            WORKING_DIRECTORY ${EMBER_BAKE_WORK_DIR}
    )

    target_sources(${EMBER_BAKE_TARGET} PRIVATE "${GENERATED_HEADER}" "${GENERATED_SOURCE}")
    target_include_directories(${EMBER_BAKE_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    # message(FATAL_ERROR "ember_bake is currently not implemented")
endfunction(ember_bake)


function(compile_spirv_shader SOURCE_FILE OUTPUT_FILE)
    # option(OUTPUT_FILE "" "${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}.spv")

    if(${DEBUG_EMBER_MODULE})
        message("SHADER_FILE : ${OUTPUT_FILE}")
    endif()

    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND glslc ${SOURCE_FILE} -o ${OUTPUT_FILE}
            MAIN_DEPENDENCY ${SOURCE_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # add_custom_target(
    #         ${OUTPUT_FILE}
    #         DEPENDS ${OUTPUT_FILE}
    # )
endfunction()

# - target_shaders
function(target_shaders)
    set(oneValueArgs)
    set(multiValueArgs SPIRV)
    cmake_parse_arguments(TARGET_SHADERS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # parse target variable

    list(LENGTH TARGET_SHADERS_UNPARSED_ARGUMENTS TARGET_SHADERS_UNPARSED_LENGTH)

    if (${TARGET_SHADERS_UNPARSED_LENGTH} LESS 1)
        message(FATAL_ERROR "No target specified for target_shaders")
    elseif(${TARGET_SHADERS_UNPARSED_LENGTH} GREATER 1)
        list(SUBLIST TARGET_SHADERS_UNPARSED_ARGUMENTS 1 ${TARGET_SHADERS_UNPARSED_LENGTH}-1 TARGET_SHADERS_UNPARSED_EXTRA)
        message(FATAL_ERROR "Unexpected arguments provided to call of target_shaders : ${TARGET_SHADERS_UNPARSED_EXTRA}")
    endif()

    list(GET TARGET_SHADERS_UNPARSED_ARGUMENTS 0 TARGET_SHADERS_TARGET)

    # debug printing

    if(${DEBUG_EMBER_MODULE})
        message("TARGET : ${TARGET_SHADERS_TARGET}")
        message("SPIRV Shaders: ${TARGET_SHADERS_SPIRV}")
        message("DXIL Shaders: ${TARGET_SHADERS_DXIL}")
    endif()

    # compile and bake shader

    foreach(SHADER ${TARGET_SHADERS_SPIRV})
        set(COMPILED_SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.spv")
        compile_spirv_shader(${SHADER} ${COMPILED_SHADER})

        ember_bake(${TARGET_SHADERS_TARGET} FILE ${COMPILED_SHADER})

        # add_dependencies(${TARGET_SHADERS_TARGET} "${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.spv")
    endforeach()

    # message(FATAL_ERROR "target_shaders is currently not implemented")
endfunction(target_shaders)