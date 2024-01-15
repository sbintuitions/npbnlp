include_directories(${CMAKE_SOURCE_DIR}/io/include)
include_directories(${CMAKE_SOURCE_DIR}/rd/include)
include_directories(${CMAKE_SOURCE_DIR}/lm/include)
include_directories(${CMAKE_SOURCE_DIR}/ma/include)
include_directories(${CMAKE_SOURCE_DIR}/tg/include)
include_directories(${CPLUS_INCLUDE_PATH})
link_directories(${CMAKE_SOURCE_DIR}/io/src)
link_directories(${CMAKE_SOURCE_DIR}/rd/src)
link_directories(${CMAKE_SOURCE_DIR}/lm/src)
link_directories(${CMAKE_SOURCE_DIR}/tg/src)
link_directories(${LIBRARY_PATH})

add_executable(ch chunking.cc clattice.cc nnpylm.cc)
add_executable(ne ne.cc clattice.cc nphsmm.cc nnpylm.cc)
add_executable(tg tg.cc hlattice.cc ihmm.cc)
find_package(Boost COMPONENTS serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

target_compile_options(ch PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
target_compile_options(ne PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
	target_link_libraries(ch PUBLIC OpenMP::OpenMP_CXX io rd lm tn icuuc ${Boost_LIBRARIES})
	target_link_libraries(ne PUBLIC OpenMP::OpenMP_CXX io rd lm tn icuuc ${Boost_LIBRARIES})
	target_link_libraries(tg PUBLIC OpenMP::OpenMP_CXX io rd lm tn icuuc ${Boost_LIBRARIES})
else()
	target_link_libraries(ch PUBLIC io rd lm tn icuuc ${Boost_LIBRARIES})
	target_link_libraries(ne PUBLIC io rd lm tn icuuc ${Boost_LIBRARIES})
	target_link_libraries(tg PUBLIC io rd lm tn icuuc ${Boost_LIBRARIES})
endif()

# for ICU
target_link_directories(ch PUBLIC /usr/local/lib)
target_link_directories(ne PUBLIC /usr/local/lib)
target_link_directories(tg PUBLIC /usr/local/lib)
#find_package(FindICU)
#if (ICU_FOUND)
#	target_include_directories(ma PUBLIC ICU_INCLUDE_DIRS)
#	target_link_libraries(ma PUBLIC ICU_LIBRARIES)
#endif()
