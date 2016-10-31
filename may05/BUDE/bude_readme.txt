bude_readme.txt - readme file for the BUDE Button Disambiguation Example.

Mick West 2005
email game@mickwest.com

See http://mickwest.com/PlayerControl for more information.

To run the executable  (release\bude.exe) you need the directX 9.0c runtime, or higher, which you can download from:

http://www.microsoft.com/windows/directx/default.aspx

In order to compile the code, you need the Microsoft DirectX 9.0c SDK, and it's currently compiling with Microsoft Visual C++.

The code compises of two source files:

vector2.cpp - a simple 2d vector library
bude.cpp - everything else, including a directX framework.

Bude is a very simple implementation of a 2D platform game, intended simply to demonstate the usefulness of log files and visual state graphs in developing and debugging player control.

The game is intended to be used with a gamepad.  With a "Standard" pc gamepad such as the logitech dual action, the controls are:

D-Pad = move left/right   & scroll graph

Button 10 = Pause/Unpause
Button 2 = Jump  & zoom in on graph
Button 6 = Crouch/Ground Pound
Button 4 = Jetpack & zoom out

With a Playstation 2 Joystick adapter, the buttons come out slightly differently

Select = Pause/Unpause
Circle =  Jump  & zoom in on graph
Square = Jetpack & zoom out
R2     = Crouch/Ground Pound

The code is mostly decoupled from DirectX, and should be pretty compiler independent. So it should be a fairly trivial exercise to get it to work under OpenGL and/or Linux, or even to any console platform.  If you do this, or port it to any other compiler, please let me know at game@mickwest.com.