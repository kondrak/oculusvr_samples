Oculus Rift DK2 IR tracking camera bounds renderer
================

Sample application with rendering code for the DK2 tracking camera bounding box (frustum). The application imitates bounds rendering behavior as seen in the Demo Scene. Tested and compiled with SDK 0.4.4.

![Screenshot](http://kondrak.info/images/vrcam_bb2.png?raw=true)
![Screenshot](http://kondrak.info/images/vrcam_bb1.png?raw=true)

Usage
-----
Run <code>CameraBounds.exe</code>

Press R while "ingame" to recenter tracking position.

How to build
-------
The application was built using VS2013. It should work out of the box with SDK 0.4.4. To compile, you need to set a OCULUS_SDK environment variable which points to the root directory of your Oculus SDK.

Dependencies
-------
This project uses following external libraries:

- GLee library for extensions (c)2011 Ben Woodhouse
- stb_image library for image handling (c) Sean Barret
- SDL2 library for window/input 
