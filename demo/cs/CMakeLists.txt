file(GLOB sources *.cpp)

set(name cs)
add_executable(${name} ${sources})

# libfts_user.soができたらこちらを使う
target_link_libraries(${name} fts_cs ${COMMON_LIBS})
#target_link_libraries(${name} ${COMMON_LIBS})
