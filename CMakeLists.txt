cmake_minimum_required(VERSION 3.10)

project(ascii2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O1")
find_library(opencv OPENCV)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_BUILD_TYPE Debug)

set(EXTERN_LIBS
  ${OpenCV_LIBS}
  )

set(EXTERN_HEADERS
  include/
  src/
  )
  
set(MAIN
  src/init.cpp
  )

set(SRC
  src/scores.cpp
  src/img_sample.cpp
  src/match.cpp
  )

add_executable(ascii2 ${MAIN} ${SRC})
target_include_directories(ascii2 PUBLIC include ${EXTERN_HEADERS})
target_link_libraries(ascii2 ${EXTERN_LIBS})	
