mg_add_sub_source_dirs (
    agg api
    )

set (MGPLUS_LIBNAME ${MGPLUS_NAME}${LIB_SUFFIX})

include_directories (
    "${CMAKE_CURRENT_SOURCE_DIR}/agg"
    "${CMAKE_CURRENT_SOURCE_DIR}/agg/font_freetype"
    )

################ make & install library #####################
if (NOT disable_shared)
    # shared
    add_library (${MGPLUS_LIBNAME} SHARED ${_source_list_})
    set_target_properties (${MGPLUS_LIBNAME} PROPERTIES VERSION ${MGPLUS_VERSION} SOVERSION ${MGPLUS_SOVERSION})
else ()
    # static
    add_library (${MGPLUS_LIBNAME} STATIC ${_source_list_})
endif (NOT disable_shared)
set_target_properties (${MGPLUS_LIBNAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries (${MGPLUS_LIBNAME} ${PC_LIBS_LIST})
install (TARGETS ${MGPLUS_LIBNAME}
    LIBRARY DESTINATION lib     # for .so  (dynamic library)
    ARCHIVE DESTINATION lib     # for .lib (import library)
    RUNTIME DESTINATION bin     # for .dll (dynamic library)
    )
