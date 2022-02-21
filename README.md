# Custom Engine


## What is it

This project is an attempt to create a custom **OpenGL ECS engine**.  
This project is build with Visual Studio, using the *Visual Studio* ***Solutions***.


## Projects

For now I'm reimplementing the library (see [Credits](#Credits)). Once done I'll migrate some old projects.

### Mesh viewer

This project is a real time mesh renderer.

[![Online render video](https://img.youtube.com/vi/csA20fUCOt8/0.jpg)](https://youtu.be/csA20fUCOt8)

### Ray tracer

This project is the real time implementation of a ray tracer.

[![Ray tracer video](https://img.youtube.com/vi/ZRVq3qCNzlI/0.jpg)](https://youtu.be/ZRVq3qCNzlI)


## Configure External ressources

### With the pre config

* Download Binaries
  * [GLEW](http://glew.sourceforge.net/index.html)
  * [GLFW](https://www.glfw.org/download.html)
  * [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
* Create a folder named ***External Ressources*** in the same root folder as this repos (one folder above the VS-solution).
* Put folders in this directory named ***GLEW***, ***GLFW*** and ***Eigen***. This should be enough, the VS solution should handle the rest.

### Redoing it manually

* Create Empty VS project and create *.cpp* file
* *project > properties*
  * *configurations : all configurations*
  * *platform : same as downloaded libraries*
  * *C/C++ > General > Additional Include Directories*
    * `$(path_to_glfw)\GLFW\include;`
    * `$(path_to_glew)\GLEW\include;`
    * `$(path_to_eigen)\Eigen\`
  * *Linker > General > Additional Library Directories*
    * `$(path_to_glfw)\GLFW\lib-vc2022;`
    * `$(path_to_glew)\GLEW\lib\Release\x64;`
  * *Linker > Input > Additional Dependencies*
    * `opengl32.lib;`
    * `glew32.lib;`
    * `glfw3.lib;`

From : [Modern OpenGL 3.0+ Visual Studio 2017 [SETUP] GLFW and GLEW on Windows](https://www.youtube.com/watch?v=gCkcP0GcCe0)


## Credits

This code is my own.

It is greatly inspired by *Jean Claude Iehl*'s [***gkit2light***](https://perso.univ-lyon1.fr/jean-claude.iehl/Public/educ/M1IMAGE/html/index.html) which was a learning support in Uni.
It's basically a super light version of it coded from scratch for me to learn every aspect of OpenGL coding, use modern GL libraries and have personal project for myself.
It is planned that I graduately migrate my Uni work on *gkit2light* to this public repos.
