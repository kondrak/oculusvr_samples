Oculus Rift mirror modes
================

This application introduces different ways to mirror the Oculus Rift output to a window which can be used for debugging. Press 'M' while running the program to toggle between:
- standard mirror with distortion
- single-eye mirror (useful for detecting stereoscopic bugs)
- non-distorted mirror for convenience

A green rectangle will determine which eye is currently rendered in single eye mode.

![Screenshot](vr_mirror1.png?raw=true)
![Screenshot](vr_mirror2.png?raw=true)

Usage
-----
Run <code>MirrorModes.exe</code>

Press SPACE while "ingame" to recenter tracking position. 'M' toggles between different mirror modes.

How to build
-------
The application was built using VS2015. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
