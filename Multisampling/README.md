Oculus Rift OpenGL Multisampling
================

Example application displaying OpenGL multisampling (MSAA) capabilities with Oculus Rift.

![Screenshot](vr_multisample.png?raw=true)

Usage
-----
Run <code>Multisampling.exe</code>

Press SPACE while "ingame" to recenter tracking position. 'M' turns MSAA on/off.

How to build
-------
The application was built using VS2015. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
