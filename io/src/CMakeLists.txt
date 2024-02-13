include_directories(${CMAKE_SOURCE_DIR}/io/include)
add_executable(test test.cc io.cc)
set(CMAKE_CXX_FLAGS "-std=c++17 -O3")
set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_USE_MULTITHREAD ON)
find_package(Boost COMPONENTS serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_executable(word_test word_test.cc io.cc word.cc)
add_executable(sentence_test sentence_test.cc io.cc word.cc sentence.cc)
add_library(io SHARED io.cc word.cc sentence.cc chunk.cc nsentence.cc nio.cc tree.cc)
target_compile_options(io PUBLIC
	$<$<CONFIG:Release>:-O3 -std=c++17>
	$<$<CONFIG:Debug>:-g -std=c++17>
	)
target_link_libraries(io PUBLIC ${Boost_LIBRARIES})
target_link_libraries(word_test PUBLIC ${Boost_LIBRARIES})
target_link_libraries(sentence_test PUBLIC ${Boost_LIBRARIES})
target_link_libraries(test PUBLIC ${Boost_LIBRARIES})
