# RT_lite
A collection of lighting algorithms for real-time rendering with OpenGL

![fig_1](img1.png)


## 1. GOALS AND CONTENT


RT_lite is a light framework for advanced real-time rendering.
This code is developed in C++ and OpenGL + GLSL, with a minimal amount of external dependencies (see 3. below).

This program implements a collection of algorithms and contains code samples from various sources (see 2. below) that has been adapted / re-written to provide a consistent framework.

The goal is to provide a demo with minimalistic GUI to demonstrates various real-time rendering algorithms, for academic purpose.


This program includes:
* Cook-Torrance lighting model [1]
* PBR textures [1]
* Shadow mapping [2] [3]
* Environment mapping [4]
* Simulate light transmission [5] [6]
* Texture-space diffusion [5] [6]
* Screen-Space Ambient Occlusion [7] [8]
* Screen-Space Directional Occlusion [9] [10]
* ...


## 2. SOURCES


Sources are online tutorials, as-well-as programs developed by students supervised by the IGS group (dpt. of computer science, University of Innsbruck, Austria) for their Master or Bachelor thesis.
Sources are mentioned as commentaries in the codes, whenever one of these algorithms is used.

* [1] https://learnopengl.com/PBR/Lighting
* [2] https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
* [3] http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
* [4] https://learnopengl.com/Advanced-OpenGL/Cubemaps
* [5] A. Brandstätter, "Real-time rendering of realistic surfaces using subsurface scattering". Bsc. thesis, university of Innsbruck dpt. of computer science, 2020.  
	  https://gitlab.com/annabrandy/approx-subsurface-scattering
* [6] S. Green, “Real-Time Approximations to Subsurface Scattering”. GPU Gems 1, chap 16.   
	  https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch16.html
* [7] https://learnopengl.com/Advanced-Lighting/SSAO
* [8] M. Albl, "Advanced Screen-Space Ambient Occlusion for Real-time rendering". Bsc. thesis, university of Innsbruck dpt. of computer science, 2021.
* [9] T. Ritschel et al., "Approximating dynamic global illumination in image space". In Proceedings of the 2009 Symposium on Interactive 3D Graphics and Games (I3D '09).  
	  https://www.in.tu-clausthal.de/fileadmin/homes/CG/data_pub/paper/SSDO_i3D09.pdf
* [10] J. Huber, "Enhancing Visual Rendering for Interactive AR". Msc. thesis, university of Innsbruck dpt. of computer science, 2019.
* ...


## 3. EXTERNAL DEPENDENCIES


All external dependencies are open-source libraries.   
For convenience, they are already provided in the "RT_lite/external" folder.


* [GLEW (The OpenGL Extension Wrangler Library)](http://glew.sourceforge.net/)
  
* [GLM (OpenGL Mathematics)](https://glm.g-truc.net/0.9.9/)

* [GLFW (Graphics Library Framework)](https://www.glfw.org/)

* [Dear ImGui (Immediate-mode Graphical User Interface)](https://github.com/ocornut/imgui)

* [LodePNG](https://lodev.org/lodepng/)  
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
* [CMake]( https://cmake.org/ )

Use CMake to generate a project/makefile, compile, and run !
