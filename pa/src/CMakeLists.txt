include_directories(${CMAKE_SOURCE_DIR}/io/include)
include_directories(${CMAKE_SOURCE_DIR}/rd/include)
include_directories(${CMAKE_SOURCE_DIR}/lm/include)
include_directories(${CMAKE_SOURCE_DIR}/pa/include)
include_directories(${CPLUS_INCLUDE_PATH})
link_directories(${CMAKE_SOURCE_DIR}/io/src)
link_directories(${CMAKE_SOURCE_DIR}/rd/src)
link_directories(${CMAKE_SOURCE_DIR}/lm/src)
link_directories(${LIBRARY_PATH})

add_executable(pa pa.cc ipcfg.cc cyk.cc)
add_library(gr SHARED cyk.cc ipcfg.cc)
find_package(Boost COMPONENTS serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

target_compile_options(pa PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
	target_link_libraries(pa PUBLIC OpenMP::OpenMP_CXX io rd lm icuuc ${Boost_LIBRARIES})
else()
	target_link_libraries(pa PUBLIC io rd lm icuuc ${Boost_LIBRARIES})
endif()

# for ICU
target_link_directories(pa PUBLIC /usr/local/lib)
#find_package(FindICU)
#if (ICU_FOUND)
#	target_include_directories(ma PUBLIC ICU_INCLUDE_DIRS)
#	target_link_libraries(ma PUBLIC ICU_LIBRARIES)
#endif()
