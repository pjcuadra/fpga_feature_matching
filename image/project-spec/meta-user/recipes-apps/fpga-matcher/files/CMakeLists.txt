# MIT License
#
# Copyright (c) 2017 Pedro Cuadra
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Define project
project(myapp)

# Define required cmake version
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB XEUCLIDIAN_SRC src/xeuclidean/*.c)
add_library(xeuclidean STATIC ${XEUCLIDIAN_SRC})

file(GLOB DMA_SRC src/dma/*.c)
message(STATUS "${DMA_SRC}")
add_library(dma STATIC ${DMA_SRC})

# Create Testing executable
add_executable(fpga-matcher
  main.cpp
)
# Link to openCV
target_link_libraries(fpga-matcher xeuclidean)
target_link_libraries(fpga-matcher dma)

install(TARGETS fpga-matcher xeuclidean dma
	RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
  PRIVATE_HEADER DESTINATION include)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include DESTINATION . FILES_MATCHING PATTERN "*.h")
