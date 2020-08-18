Oculus Rift Vive-style tracker chaperone
================

Sample program demonstrating a HTC Vive-like chaperone for Oculus Rift tracker camera. Tracking planes will lit up once the users move their heads too close to them.

[![Screenshot](vr_chaperone.png?raw=true)](https://www.youtube.com/watch?v=GoWwFa_TDGM)

Usage
-----
Run <code>TrackerChaperone.exe</code>

How to build
-------
The application was built using VS2015. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
