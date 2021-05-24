# RT_lite
A collection of lighting algorithms for real-time rendering with OpenGL


## 1. GOALS AND CONTENT


RT_lite is a light framework for advanced real-time rendering.
This code is developed in C++ and OpenGL + GLSL, with a minimal amount of external dependencies (see 3. below).

This program implements a collection of algorithms and contains code samples from various sources (see 2. below) that has been adapted / re-written to provide a consistent framework.

The goal is to provide a simple program with minimalistic GUI to demonstrates various real-time rendering algorithms, and to provide a collection of comprehensible algorithms for academic purpose.




## 2. SOURCES

TODO



## 3. EXTERNAL DEPENDENCIES


All external dependencies are open-source libraries. 
For convenience, they are already provided in the "RT_lite/external" folder.


* GLEW (The OpenGL Extension Wrangler Library)
  http://glew.sourceforge.net/
  
* GLM (OpenGL Mathematics)
  https://glm.g-truc.net/0.9.9/

* GLFW (Graphics Library Framework)
  https://glm.g-truc.net/0.9.9/

* Dear ImGui (Immediate-mode Graphical User Interface)
  https://github.com/ocornut/imgui

* LodePNG 
  https://lodev.org/lodepng/
  https://github.com/lvandeve/lodepng




## 4. DATA


The folder "RT_lite/models" contains several ready-to-use meshes and textures.
These files come from various sources and have sometime been modified to fit our requirements.

Cubemaps: http://www.humus.name
Meshes: http://graphics.stanford.edu/data/3Dscanrep/
PBR textures and models : https://artisaverb.info/Cerberus.html

The only file formats supported are Wavefront (.obj) for meshes, and PNG for textures.
For optimal rendering quality the meshes should be watertight, and should contain UV coords generated through mesh unwrapping (with hidden seams, as-much-as-possible).



## 5. COMPILATION


RT_lite is provided as a ready-to-build folder with a CMakeList. 

The only requirement are:
* C++ compiler
* OpenGL 3.0+
* CMake ( https://cmake.org/ )

Use CMake to generate a project/makefile, compile, and run !