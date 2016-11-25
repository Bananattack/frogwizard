# frogwizard
a little platform game thing with a cute frog for the Arduboy + PC.

#Building

##Arduboy

You will need the Arduino IDE, the Arduboy Library, and the Arduboy board description. Tips for setting all of this up [can be found here](http://community.arduboy.com/t/arduboy-quick-start-guide/725).

Open `code/frogwizard/frogwizard.ino` in the Arduino IDE. Set the appropriate port + board information, then Verify and Upload to the Arduboy.

##Windows

Open `code/frogwizard.sln` in Visual Studio 2015. Build the solution and run the program with the debugger.

Alternatively, you can build the solution, and then run the executable from the output folder. It should be a portable exe with all dependencies statically included. There's build setups for Debug and Release for both x86 and x64, pick whatever works best for you.

##Mac / Linux / etc

TODO. hopefully pretty painless, since this already uses SDL. Just need to setup the appropriate build files: xcode project for Mac, GNU Makefile for Linux. I refuse to use CMake or SCons.
