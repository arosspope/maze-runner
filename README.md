# Maze Runner
 
This project uses a PIC-16F877A micro-controller and Xilinx XC9572XL CPLD to navigate
an 'iRobot Create' through a maze, finding 'victims' and alerting the operator.
The main requirements of the project were as follows:
+ Operate autonomously during the mission. There should be no user intervention or control
other than pressing the start button.
+ Search for and locate ‘victims’ within the maze. The victims will be marked with the home-
base beacons, but their location is otherwise unknown.
+ After locating a victim, play a song to indicate that the victim has been found. The robot
should resume exploring the maze to find the second victim.
+ After finding the second victim, the robot should return to its starting location by taking the
shortest path back.
+ Avoid walls and respond appropriately to virtual walls while carrying out the tasks required
for each mission.

A top down representation of the maze space can be viewed [here](maze.png).

## Hardware

Using the integrated PIC-16F877A and XC9572XL CPLD chip on the D5 Series DSX Kit, a variety
of peripheral devices were connected through the PORTs located on the board. These devices
include:
+ Infrared sensor - Used to measure distances
+ Stepper Motor - The IR sensor is mounted to the stepper motor which allows for distance readings in all directions
+ Push Buttons - Used to signal when main operation should commence
+ LCD Display - Used to display output from the program (primarily used for debugging purposes)

## Software

## Building the project

This project is best compiled using:
+ [MPLAB X IDE](http://www.microchip.com/mplab/mplab-x-ide)
+ [XC8 Pro-Compiler](http://www.microchip.com/mplab/compilers)
