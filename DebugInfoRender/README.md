Oculus Rift Debug Data Rendering
================

Rendering HMD debug data similar to the manner it's done in OculusWorldDemo. This project has been obsoleted after the SDKs introduced built-in debug overlays.

![Screenshot](http://kondrak.info/images/vr_debug.png?raw=true)

Usage
-----
Run <code>DebugInfoRender.exe</code>

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
