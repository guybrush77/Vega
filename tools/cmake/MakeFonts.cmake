function(make_fonts ttf_files prefix output)
    foreach(ttf_file ${ttf_files})
        set(ttf_resource ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${ttf_file}.cpp)
        add_custom_command(
            OUTPUT ${ttf_resource}
            COMMAND make-resource --resource ${prefix}${ttf_file} --input ${CMAKE_CURRENT_SOURCE_DIR}/${ttf_file} --output ${ttf_resource}
            DEPENDS make-resource ${ttf_file}
            COMMENT "Making resource ${ttf_resource}"
        )
        get_property(file_location SOURCE ${ttf_resource} PROPERTY LOCATION)
        list(APPEND result ${file_location})
    endforeach()
    set(${output} ${result} PARENT_SCOPE)
endfunction()
