
find_package(HDF5 REQUIRED)

LIST(APPEND sources "src/cheetah-pal.cpp")
LIST(APPEND sources "src/hdf5-reader.cpp")

include_directories("include")
include_directories(${CHEETAH_INCLUDES})
include_directories(${CHEETAH_INCLUDES}/cheetah_extensions_yaroslav)
include_directories(${HDF5_INCLUDE_DIRS})

add_executable(cheetah-pal ${sources})
add_dependencies(cheetah-pal cheetah)
target_link_libraries(cheetah-pal ${CHEETAH_LIBRARY} ${HDF5_HL_LIBRARIES} ${HDF5_C_LIBRARIES} )
install(TARGETS cheetah-pal
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
