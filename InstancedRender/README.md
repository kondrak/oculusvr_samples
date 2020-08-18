Oculus Rift OpenGL instanced rendering
================

This is a demonstration of instanced VR rendering in OpenGL. Contrary to other examples, this demo uses single render target for both eyes. Geometry shader capable graphics card with OpenGL 4.1+ support is required.

![Screenshot](vr_instanced.png?raw=true)

Usage
-----
Run <code>InstancedRender.exe</code>

Press SPACE while "ingame" to recenter tracking position.  Press R during the demo to toggle between standard (one scene draw per eye, red quads) and instanced (single draw for both eyes, green quads) rendering. Notice the performance difference between the two!

How to build
-------
The application was built using VS2015. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
