# check CMAKE version
cmake_minimum_required(VERSION 3.0)

# create project
project(RT_lite)

# add files
set(SRCS
	src/main.cpp
	src/trimesh.cpp
	src/drawablemesh.cpp
    )
    
set(HEADERS
	src/tools.h
	src/trimesh.h
	src/drawablemesh.h
    )
	

# Add source directories
aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/src" PROJECT_SRCS)
	
################################# EXTERNAL LIBS ######################

# Add include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src" )

# Define variable for linked libraries
#set(PROJECT_LIBRARIES)

# OpenGL
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
  include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
  #set(PROJECT_LIBRARIES ${PROJECT_LIBRARIES} ${OPENGL_LIBRARIES})
endif(OPENGL_FOUND)


# GLEW (download binaries for windows)
set(GLEW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glew-2.1.0")
include_directories(${GLEW_DIR}/include)
link_directories(${GLEW_DIR}/lib/Release/x64)
# use glew32s only
SET(GLEW_LIBS glew32s.lib)
add_definitions(-DGLEW_STATIC -DGLEW_NO_GLU)


# GLFW (to compile before)
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-3.3.8")
include_directories(${GLFW_DIR}/include)
link_directories(${GLFW_DIR}/build/src/Release)
SET(GLFW_LIBS glfw3.lib)


# GLM (Header only!)
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/external/glm")


# ImGui (just add sources to project
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
aux_source_directory("${IMGUI_DIR}" PROJECT_SRCS)
# new: add appropriate backends
set(IMGUI_BCK "${IMGUI_DIR}/backends/imgui_impl_glfw.h"
			  "${IMGUI_DIR}/backends/imgui_impl_opengl3.h"
			  "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
			  "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")					
include_directories(SYSTEM "${IMGUI_DIR}")
include_directories(SYSTEM "${IMGUI_DIR}/backends")

# lodepng
aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/external/lodepng" PROJECT_SRCS)
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/external/lodepng")


# Add executable for project
add_executable(${PROJECT_NAME} ${PROJECT_SRCS} ${SRCS} ${HEADERS} ${IMGUI_BCK})


target_link_libraries(${PROJECT_NAME} ${GLFW_LIBS} ${GLEW_LIBS} ${OPENGL_LIBRARIES})

# Install executable
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

