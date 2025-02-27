include_directories(${CMAKE_SOURCE_DIR}/io/include)
include_directories(${CMAKE_SOURCE_DIR}/rd/include)
include_directories(${CMAKE_SOURCE_DIR}/lm/include)
include_directories(${CMAKE_SOURCE_DIR}/re/include)
include_directories(${CMAKE_SOURCE_DIR}/dt/include)
include_directories(${CPLUS_INCLUDE_PATH})
link_directories(${CMAKE_SOURCE_DIR}/io/src)
link_directories(${CMAKE_SOURCE_DIR}/rd/src)
link_directories(${CMAKE_SOURCE_DIR}/lm/src)
link_directories(${LIBRARY_PATH})

#add_executable(kym re.cc hsmm.cc ylattice.cc)
#add_library(re SHARED hsmm.cc ylattice.cc)
#add_executable(idx index.cc trie.cc)
add_executable(idx index.cc)
#add_executable(cps commom_prefix_search.cc trie.cc)
add_executable(cps common_prefix_search.cc)
#add_executable(koyomi koyomi.cc hsmm.cc ylattice.cc trie.cc)
add_executable(kym koyomi.cc hsmm.cc ylattice.cc)
#target_compile_options(kym PUBLIC
target_compile_options(idx PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
target_compile_options(cps PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
target_compile_options(kym PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
	target_link_libraries(idx io)
	target_link_libraries(cps io)
	target_link_libraries(kym PUBLIC OpenMP::OpenMP_CXX io rd lm)
	#target_link_libraries(re PUBLIC OpenMP::OpenMP_CXX io rd lm)
else()
	target_link_libraries(idx io)
	target_link_libraries(cps io)
	target_link_libraries(kym PUBLIC io rd lm)
	#target_link_libraries(re PUBLIC io rd lm)
endif()
