Oculus Rift minimum OpenGL setup
================

A minimum setup application for Oculus Rift development with OpenGL and SDL2 as window manager. While the entire code could be squashed into a single file, a separate OculusVR class exists to handle all OVR initialization and processing. Along with it comes a separate ShaderManager with Shader class for cleaner shader loading. The application also provides head tracking recenter functionality at a keystroke.

![Screenshot](vr_minimum.png?raw=true)

Usage
-----
Run <code>MinimumOpenGL.exe</code>

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
