include_directories(${CMAKE_SOURCE_DIR}/io/include)
include_directories(${CMAKE_SOURCE_DIR}/rd/include)
include_directories(${CMAKE_SOURCE_DIR}/lm/include)
include_directories(${CMAKE_SOURCE_DIR}/ma/include)
include_directories(${ICU_INCLUDE_DIRS})
link_directories(${CMAKE_SOURCE_DIR}/io/src)
link_directories(${CMAKE_SOURCE_DIR}/rd/src)
link_directories(${CMAKE_SOURCE_DIR}/lm/src)
link_directories(${ICU_LIBRARIES})

add_executable(ws tokenize.cc npylm.cc lattice.cc)
add_executable(ma ma.cc phsmm.cc npylm.cc lattice.cc)
add_library(tn SHARED npylm.cc phsmm.cc lattice.cc)
find_package(Boost COMPONENTS serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

target_compile_options(ws PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
target_compile_options(ma PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
	target_link_libraries(ws PUBLIC OpenMP::OpenMP_CXX io rd lm icuuc ${Boost_LIBRARIES})
	target_link_libraries(ma PUBLIC OpenMP::OpenMP_CXX io rd lm icuuc ${Boost_LIBRARIES})
	target_link_libraries(tn PUBLIC OpenMP::OpenMP_CXX io rd lm icuuc ${Boost_LIBRARIES})
else()
	target_link_libraries(ws PUBLIC io rd lm icuuc ${Boost_LIBRARIES})
	target_link_libraries(ma PUBLIC io rd lm icuuc ${Boost_LIBRARIES})
	target_link_libraries(tn PUBLIC io rd lm icuuc ${Boost_LIBRARIES})
endif()

# for ICU
target_link_directories(ws PUBLIC /usr/local/lib)
target_link_directories(ma PUBLIC /usr/local/lib)
target_link_directories(tn PUBLIC /usr/local/lib)
#find_package(FindICU)
#if (ICU_FOUND)
#	target_include_directories(ma PUBLIC ICU_INCLUDE_DIRS)
#	target_link_libraries(ma PUBLIC ICU_LIBRARIES)
#endif()
