CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(final CXX)

IF(NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE Release)
ENDIF()

ADD_SUBDIRECTORY(deps/vecmath)

SET(final_SOURCES
        src/image.cpp
        src/main.cpp
        src/mesh.cpp
        src/scene_parser.cpp)

SET(final_INCLUDES
        include/camera.hpp
        include/group.hpp
        include/hit.hpp
        include/image.hpp
        include/light.hpp
        include/material.hpp
        include/mesh.hpp
        include/object3d.hpp
        include/plane.hpp
        include/ray.hpp
        include/scene_parser.hpp
        include/sphere.hpp
        include/transform.hpp
        include/triangle.hpp
        include/parameter.hpp
        include/timer.hpp
        include/render.hpp
)

SET(CMAKE_CXX_STANDARD 11)


# 查找OpenMP包
FIND_PACKAGE(OpenMP REQUIRED)
IF(OPENMP_FOUND)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF()


ADD_EXECUTABLE(${PROJECT_NAME} ${final_SOURCES} ${final_INCLUDES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath ${OpenMP_CXX_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include)
