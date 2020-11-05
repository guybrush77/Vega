function(compile_shaders glsl_files prefix output)
    find_program(Vulkan_GLSLC_EXECUTABLE NAMES glslc PATHS "$ENV{VULKAN_SDK}/Bin" REQUIRED)
    foreach(glsl_file ${glsl_files})
        set(glsl_binary ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${glsl_file}.spv)
        add_custom_command(
            OUTPUT ${glsl_binary}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${glsl_file} -O -o ${glsl_binary}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${glsl_file}
            COMMENT "Compiling ${glsl_file}"
        )
        set(glsl_resource ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${glsl_file}.cpp)
        add_custom_command(
            OUTPUT ${glsl_resource}
            COMMAND make-resource --resource ${prefix}${glsl_file} --input ${glsl_binary} --output ${glsl_resource}
            DEPENDS make-resource ${glsl_binary}
            COMMENT "Making resource ${glsl_resource}"
        )
        get_property(file_location SOURCE ${glsl_resource} PROPERTY LOCATION)
        list(APPEND result ${file_location})
    endforeach()
    set(${output} ${result} PARENT_SCOPE)
endfunction()
