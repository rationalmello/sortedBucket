cmake_minimum_required(VERSION 3.5)

project(sortedBucket)
add_executable(bench src/bench.cpp)
target_include_directories(bench PUBLIC include)
target_compile_features(bench PUBLIC cxx_std_20)
add_subdirectory(benchmark)
target_link_libraries(bench PUBLIC benchmark::benchmark)

add_executable(test test/parity.cpp)
target_include_directories(test PUBLIC include)
target_include_directories(test PUBLIC src)
target_compile_features(test PUBLIC cxx_std_20)