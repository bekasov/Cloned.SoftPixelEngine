All of these shaders are written by Lukas Hermanns the creator of the "SoftPixel Engine"
and may be used under the terms of the "zlib/libpng" license in all of your programs.
The documentation and descriptions are inside the shader files.

If you are new in using and writing shaders you have to look inside the shaders in the
following order, because some techniques are not explained in each shader again:

 - SimpleVertexManipulation
 - Billboarding
 - ProceduralTextures
 - FurEffect
 - ...

BUGS:

 * There is one known bug: the water shader simulation GLSL shader does not run on my ATI card.
   On NVIDIA Cards it should run correct. There are no error messages the example is just incorrect
   for the OpenGL shader!