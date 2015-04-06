Oculus Rift DK2 Debug Data Rendering
================

Rendering HMD debug data similar to the manner it's done in OculusWorldDemo.

![Screenshot](http://kondrak.info/images/vr_debug.png?raw=true)

Usage
-----
Run <code>CameraBounds.exe</code>

Press SPACE while "ingame" to recenter tracking position.

How to build
-------
The application was built using VS2013. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
