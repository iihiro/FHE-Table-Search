file(GLOB sources *.cpp)

set(name user)
add_executable(${name} ${sources})

# libfts_user.soができたらこちらを使う
target_link_libraries(${name} fts_user ${COMMON_LIBS})
#target_link_libraries(${name} ${COMMON_LIBS})
