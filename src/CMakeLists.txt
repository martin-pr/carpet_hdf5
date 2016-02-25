include_directories(./)

file(GLOB sources *.cpp)

add_executable(carpet_hdf5 ${sources})
target_link_libraries(carpet_hdf5 ${LIBS})
