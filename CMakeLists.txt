cmake_minimum_required(VERSION 3.16)
project(ChessEngine LANGUAGES CXX)

# ========================
#  Compiler settings
# ========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/O2 /W4 /DNOMINMAX)
else()
    add_compile_options(-O3 -Wall -Wextra -Wshadow -pedantic -march=native)
endif()

# ========================
#  Directories
# ========================
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set(DATA_DIR ${CMAKE_SOURCE_DIR}/data)

include_directories(${SRC_DIR})
include_directories(${SRC_DIR}/AI)
include_directories(${SRC_DIR}/NNUE)
include_directories(${SRC_DIR}/utils)

# ========================
#  Source files
# ========================
file(GLOB_RECURSE ENGINE_SRC
    ${SRC_DIR}/*.cpp
)

file(GLOB_RECURSE TEST_SRC
    ${TEST_DIR}/*.cpp
)

# ========================
#  Main executable
# ========================
add_executable(chess_engine ${ENGINE_SRC})

# ========================
#  Tests
# ========================
add_executable(engine_tests ${TEST_SRC} ${ENGINE_SRC})

enable_testing()
add_test(NAME EngineTests COMMAND engine_tests)

# ========================
#  Data Files
# ========================
# Copy NNUE and book to build dir
add_custom_command(TARGET chess_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DATA_DIR}/nnue_001.nnue
        $<TARGET_FILE_DIR:chess_engine>
)

add_custom_command(TARGET chess_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DATA_DIR}/opening_book.bin
        $<TARGET_FILE_DIR:chess_engine>
)

# ========================
#  Install rules
# ========================
install(TARGETS chess_engine DESTINATION bin)
install(FILES ${DATA_DIR}/nnue_001.nnue DESTINATION bin)
install(FILES ${DATA_DIR}/opening_book.bin DESTINATION bin)
