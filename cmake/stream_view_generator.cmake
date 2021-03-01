# generate stream_view-config.cmake for other libraries

function(stream_view_generate_config)
    get_filename_component(STREAM_VIEW_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
    get_filename_component(INCLUDE_DIR "${STREAM_VIEW_DIR}/include" ABSOLUTE)

    include(CMakePackageConfigHelpers)

    configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/templates/stream_view-config.template.cmake
        ${CMAKE_CURRENT_LIST_DIR}/stream_view-config.cmake
        INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake
        PATH_VARS STREAM_VIEW_DIR INCLUDE_DIR)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_LIST_DIR}/stream_view-version.cmake
      VERSION 1.0.0
      COMPATIBILITY AnyNewerVersion)
endfunction()

stream_view_generate_config()
