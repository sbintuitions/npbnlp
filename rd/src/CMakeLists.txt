include_directories(${CMAKE_SOURCE_DIR}/io/include)
include_directories(${CMAKE_SOURCE_DIR}/rd/include)
include_directories(${CMAKE_SOURCE_DIR}/lm/include)

set(CMAKE_CXX_FLAGS "-std=c++17 -O3")
add_library(rd SHARED beta.cc generator.cc seed.cc uniform.cc poisson.cc vtable.cc)

