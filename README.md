# Notes
I will describe the layout of the code and some future ideas here later
I will attempt to update the code base to follow https://google.github.io/styleguide/cppguide.html and continue to follow it

# Things to do
* I need to tidy up my model loading, I changed how it works and am storing some things I never use
* Futhermore my model loader only loads diffuse textures from a material file, I need to load the other textures too
* I need to keep a quaternion in the camera which keeps track of the current orientation of the camera so that I can get just the rotation of the camera - Useful for cube maps

# References - sources for inspiration (rarely cut and paste)
* My camera class - [Jamie King’s Youtube](https://www.youtube.com/user/1kingja)
* My lighting, model transformations, texture mapping, skyboxes - [Anton's book](http://antongerdelan.net/opengl/#ebook)
* Loading models with assimp - [OGLdev](http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html)
* Assimp library - [Assimp](http://assimp.sourceforge.net/)
* Shader Loading - [in2gpu tutorials](http://in2gpu.com/opengl-3/)
* Introduction to OpenGL - [ACM SIGGRAPH notes](http://www.cs.unm.edu/~angel/SIGGRAPH13/An%20Introduction%20to%20OpenGL%20Programming.pdf)
* Object oriented design of VBOs, VAOs, Shaders, Renderers - [The ChernoProject](https://www.youtube.com/user/TheChernoProject)
* Cel shading - [Sun and Black Cat](http://sunandblackcat.com/tipFullView.php?l=eng&topicid=15)
* Other shading models (blinn phong, minneart, ward) - [Lossaso](http://web.cs.wpi.edu/~emmanuel/courses/cs563/S05/projects/surface_reflection_losasso.pdf) 
* For some interesting shader implementations in glsl - [Geeks3d](http://www.geeks3d.com/shader-library/)
* Particle pools - [Bob Nystrom](http://gameprogrammingpatterns.com/)
* Fresnel Effect - [Nvidia](http://developer.download.nvidia.com/CgTutorial/cg_tutorial_chapter07.html)
* Normal mapping - [OpenGL tutorial](http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/)
* Changing texture filtering over time - [OpenGL redbook](https://github.com/openglredbook/examples/blob/master/src/06-mipfilters/06-mipfilters.cpp)
* Automatic mipmap generation - [OpenGL wiki](https://www.khronos.org/opengl/wiki/Common_Mistakes#Automatic_mipmap_generation)
* Stbi image loader for textures - [Github stb](https://github.com/nothings/stb)
* Quaternion rotation calculations - [OpenGL tutorial](https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.cpp)
* Cathmull-Rom Splines - [Microsoft MVP](https://www.mvps.org/directx/articles/catmull/)