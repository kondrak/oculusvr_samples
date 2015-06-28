Oculus Rift DK2 Leap Motion 2.2.6 integration demo
================

Demo of Leap Motion integration with DK2. This program demonstrates how to track hands, gestures, how to render the skeletal outlines (using simple line segments) as well as how to display raw camera image on the screen. 

![Screenshot](http://kondrak.info/images/vr_leap1.png?raw=true)
![Screenshot](http://kondrak.info/images/vr_leap2.png?raw=true)

Usage
-----
Make sure you have the Leap.dll on your computer.

Run <code>LeapMotion.exe</code>

During the demo, swipe your hand left or right to toggle between hand skeleton rendering and camera view.

How to build
-------
The application was built using VS2013. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK and LEAP_SDK pointing to root directory of your LeapMotion SDK 2.2.6.

Dependencies
-------
This project uses following external libraries:

- GLEW OpenGL extension library
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
- Leap Motion SDK 2.2.6