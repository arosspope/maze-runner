# Maze Runner

[![maze-runner](http://img.youtube.com/vi/6FYnI7x7weY/0.jpg)](http://www.youtube.com/watch?v=6FYnI7x7weY)
> _Click on the image for a video demonstration of the robot traversing the maze_

This project uses a PIC-16F877A micro-controller and Xilinx XC9572XL CPLD to navigate an 'iRobot Create' through a maze, finding 'victims' and alerting the operator. The main requirements of the project were as follows:
+ Operate autonomously during the mission. There should be no user intervention or control other than pressing the start button.
+ Search for and locate ‘victims’ within the maze. The victims will be marked with the home-base beacons, but their location is otherwise unknown.
+ After locating a victim, play a song to indicate that the victim has been found. The robot should resume exploring the maze to find the second victim.
+ After finding the second victim, the robot should return to its starting location by taking the shortest path back.
+ Avoid walls and respond appropriately to virtual walls while carrying out the tasks required for each mission.

A top down representation of the maze space can be viewed [here](resources/maze.png).

## Hardware

Using the integrated PIC-16F877A and XC9572XL CPLD chip on the D5 Series DSX Kit, a variety of peripheral devices were connected through the PORTs located on the board. These include...
+ Infrared sensor: For measuring distance between robot and wall.
+ Stepper Motor: For controlling the direction of the infrared sensor (360 degree field of view).
+ Tactile push buttons: For operator interaction with the robot.
+ LCD Display: To alert the operator and display debug output.

## Software

The [software architecture](resources/sys-arch.JPG) of the project was designed with modularity in mind. Key functionality includes...

+ [BNT](src/BNT.h): Concerned with controlling buttons within the system (this includes debouncing).
+ [LED](src/LED.h): Controls Light Emitting Diodes.
+ [SPI](src/SPI.h): Defines an interface to the SPI available on the PIC.
+ [ADC](src/ADC.h): Defines an interface to the Analog to Digital Converter on the PIC.
+ [USART](src/USART.h): Defines an interface for serial communication (key method of communication between the iRobot and the PIC).
+ [SM](src/SM.h): Interface for Stepper Motor movement.
+ [IR](src/IR.h): Interface for obtaining distance measurements from the IR sensor.
+ [PATH](src/PATH.h): Module dedicated to calculating paths between waypoints in the maze, and tracking the robot's movement.
+ [MOVE](src/MOVE.h): Interface for robot movement (driving, rotating, checking sensors).
+ [IROBOT](src/IROBOT.h): Module dedicated for maze exploration and navigation.

## Building the project

This project is best compiled using:
+ [MPLAB X IDE](http://www.microchip.com/mplab/mplab-x-ide)
+ [XC8 Pro-Compiler](http://www.microchip.com/mplab/compilers)

### Contributors
+ Pope. A ([@arosspope](https://github.com/andrewpo456))
+ Truong. A ([@TruongAndrew](https://github.com/TruongAndrew))
+ Stewart. C ([@corstew](https://github.com/corstew))
+ Leone. K ([@kateleone](https://github.com/kateleone))
+ Lynch. J
+ Samios. L
