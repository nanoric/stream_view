set(stream_view_VERSION 3.0.0)

if (NOT stream_view_FIND_COMPONENTS)
    set(stream_view_FIND_COMPONENTS "stream_view-header-only")
    foreach (_stream_view_component ${stream_view_FIND_COMPONENTS})
        set(stream_view_FIND_REQUIRED_${_stream_view_component} 1)
    endforeach()
endif()

#set(_stream_view_include_dir "@PACKAGE_THIRD_PARTY_INCLUDES@;@PACKAGE_SRC_DIR@")
set(_stream_view_include_dir "@PACKAGE_INCLUDE_DIR@")

add_library(stream_view::stream_view-header-only INTERFACE IMPORTED)

set_target_properties(stream_view::stream_view-header-only PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_stream_view_include_dir}"
)

unset(_library_ext)
unset(_stream_view_include_dir)
