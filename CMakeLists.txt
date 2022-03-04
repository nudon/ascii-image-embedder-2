cmake_minimum_required(VERSION 3.10)

project(ascii2)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O1")
find_package(OpenCV REQUIRED)
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
find_package(Freetype REQUIRED)


set(EXTERN_LIBS
  ${OpenCV_LIBS}
  ${ImageMagick_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  )

set(EXTERN_HEADERS
  ${OpenCV_INCLUDE_DIRS}
  ${ImageMagick_Magick++_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  )
  
set(MAIN
  src/init.cpp
  )

set(SRC
  src/scores.cpp
  src/img_sample.cpp
  src/match.cpp
  src/font.cpp
  src/options.cpp
  )

set(HEADERS
  include/
  src/
  )

#quickly fixes some ImageMagick warnings, alternative fix is here
#https://stackoverflow.com/questions/38546196/how-can-i-determine-magickcore-quantum-depth-and-magickcore-hdri-enable-during-c
add_compile_definitions(MAGICKCORE_HDRI_ENABLE=0 MAGICKCORE_QUANTUM_DEPTH=16)

include_directories(${EXTERN_HEADERS})

add_executable(ascii2 ${MAIN} ${SRC})
target_include_directories(ascii2 PUBLIC include ${HEADERS})
target_link_libraries(ascii2 ${EXTERN_LIBS})	
