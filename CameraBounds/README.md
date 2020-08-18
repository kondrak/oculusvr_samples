Oculus Rift IR tracking camera bounds renderer (OpenGL)
================

Sample program with rendering code for the tracking camera bounding box (frustum) using OpenGL. The application imitates bounds rendering behavior as seen in the Demo Scene. The "core rendering" is performed in <code>src/renderer/CameraFrustum.hpp</code> and <code>src/renderer/CameraFrustum.cpp</code> which should be fairly straightforward to transfer to your own codebase.

![Screenshot](vrcam_bb2.png?raw=true)
![Screenshot](vrcam_bb1.png?raw=true)

Usage
-----
Run <code>CameraBounds.exe</code>

Press SPACE while "ingame" to recenter tracking position.

How to build
-------
The application was built using VS2015. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
