include_directories(${CMAKE_SOURCE_DIR}/io/include)
include_directories(${CMAKE_SOURCE_DIR}/rd/include)
include_directories(${CMAKE_SOURCE_DIR}/lm/include)
include_directories(${CPLUS_INCLUDE_PATH})

#set(CMAKE_CXX_FLAGS "-std=c++17 -O3")
add_executable(lm_test test.cc hpyp.cc vpyp.cc context.cc hdp.cc hdp_context.cc cache.cc chartype.cc)
add_executable(hdp_test hdp_test.cc hpyp.cc vpyp.cc context.cc hdp.cc hdp_context.cc cache.cc chartype.cc)
add_library(lm SHARED hdp.cc hpyp.cc vpyp.cc context.cc hdp_context.cc cache.cc chartype.cc)
target_compile_options(lm_test PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
target_compile_options(lm PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
link_directories(${CMAKE_SOURCE_DIR}/io/src)
link_directories(${CMAKE_SOURCE_DIR}/rd/src)
link_directories(${CMAKE_SOURCE_DIR}/lm/src)
link_directories(${LIBRARY_PATH})
find_package(Boost COMPONENTS serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries(lm_test PUBLIC OpenMP::OpenMP_CXX io rd ${Boost_LIBRARIES} icuuc)
	target_link_libraries(hdp_test PUBLIC OpenMP::OpenMP_CXX io rd ${Boost_LIBRARIES} icuuc)
	target_link_libraries(lm PUBLIC OpenMP::OpenMP_CXX io rd ${Boost_LIBRARIES} icuuc)
else()
	target_link_libraries(lm_test io rd ${Boost_LIBRARIES} icuuc)
	target_link_libraries(hdp_test io rd ${Boost_LIBRARIES} icuuc)
endif()
target_link_directories(lm_test PUBLIC /usr/local/lib ${LIBRARY_PATH})
target_link_directories(hdp_test PUBLIC /usr/local/lib ${LIBRARY_PATH})
target_link_directories(lm PUBLIC /usr/local/lib ${LIBRARY_PATH})
