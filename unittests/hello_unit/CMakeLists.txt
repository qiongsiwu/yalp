set(TESTTARGET "hello_unit")

add_executable(${TESTTARGET} hello_unit.cpp)

target_include_directories(${TESTTARGET} PRIVATE "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(${TESTTARGET} gtest_main)
add_test(NAME hello_unit COMMAND hello_unit)
