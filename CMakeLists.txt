cmake_minimum_required(VERSION "3.17")
project(exec)

set(OpenCV_DIR /usr/local/lib/cmake/opencv4)
set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# collect all source files
file(GLOB SOURCES
    "*.cpp"
)

# Suppress specific warnings for GCC and Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-write-strings)
endif()

# specify the executable target to be built
add_executable(exec ${SOURCES})

# tell it to link the executable target against OpenCV
target_link_libraries(exec ${OpenCV_LIBS})
