Oculus Rift DK2 Leap Motion 2.2.6 integration test
================

Test of Leap Motion integration with DK2.

Usage
-----
Run <code>LeapMotion.exe</code>

Press SPACE while "ingame" to recenter tracking position.

How to build
-------
The application was built using VS2013. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK and LEAP_SDK pointing to root directory of your LeapMotion SDK 2.2.6.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
