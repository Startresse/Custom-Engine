# Custom Engine

## What is it

This project is an attempt to create a custom **OpenGL ECS engine**.  
This project is build with Visual Studio, using the *Visual Studio* ***Solutions***.

## Configure openGL project


* Download Binaries
  * [GLEW](http://glew.sourceforge.net/index.html)
  * [GLFW](https://www.glfw.org/download.html)
* Create Empty VS project and create *.cpp* file
* *project > properties*
  * *configurations : all configurations*
  * *platform : same as downloaded libraries*
  * *C/C++ > General > Additional Include Directories*
    * `$(path_to_glfw)\GLFW\include;`
    * `$(path_to_glew)\GLEW\include;`
  * *Linker > General > Additional Library Directories*
    * `$(path_to_glfw)\GLFW\lib-vc2022;`
    * `$(path_to_glew)\GLEW\lib\Release\x64;`
  * *Linker > Input > Additional Dependencies*
    * `opengl32.lib;`
    * `glew32.lib;`
    * `glfw3.lib;`

From : [Modern OpenGL 3.0+ Visual Studio 2017 [SETUP] GLFW and GLEW on Windows](https://www.youtube.com/watch?v=gCkcP0GcCe0)
