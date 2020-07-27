file(GLOB sources *.cpp)

set(name make_table_for_two_input)
add_executable(${name} ${sources})

target_link_libraries(${name})
