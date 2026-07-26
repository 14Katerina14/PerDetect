function(securezone_copy_mongodb_runtime target_name)
    if(NOT WIN32 OR NOT TARGET ${target_name})
        return()
    endif()

    add_custom_command(
        TARGET ${target_name}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${target_name}>
            $<TARGET_FILE_DIR:${target_name}>
        COMMAND_EXPAND_LISTS
    )

    set(securezone_mongodb_package_root "")
    if(DEFINED bsoncxx_DIR)
        get_filename_component(
            securezone_mongodb_package_root
            "${bsoncxx_DIR}/../.."
            ABSOLUTE
        )
    elseif(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(
            securezone_mongodb_package_root
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
        )
    endif()

    if(securezone_mongodb_package_root)
        set(
            securezone_mongodb_runtime_path
            "${securezone_mongodb_package_root}/$<IF:$<CONFIG:Debug>,debug/bin,bin>"
        )
        add_custom_command(
            TARGET ${target_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${securezone_mongodb_runtime_path}"
                $<TARGET_FILE_DIR:${target_name}>
        )
    endif()
endfunction()

function(securezone_set_mongodb_test_runtime test_name)
    if(NOT WIN32)
        return()
    endif()

    set(securezone_mongodb_package_root "")
    if(DEFINED bsoncxx_DIR)
        get_filename_component(
            securezone_mongodb_package_root
            "${bsoncxx_DIR}/../.."
            ABSOLUTE
        )
    elseif(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(
            securezone_mongodb_package_root
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
        )
    endif()

    if(securezone_mongodb_package_root)
        set(
            securezone_mongodb_runtime_path
            "${securezone_mongodb_package_root}/$<IF:$<CONFIG:Debug>,debug/bin,bin>"
        )
        set_tests_properties(
            ${test_name}
            PROPERTIES
                ENVIRONMENT_MODIFICATION
                    "PATH=path_list_prepend:${securezone_mongodb_runtime_path}"
        )
    endif()
endfunction()
