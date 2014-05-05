Route Map Creation for Train Simulator 
======================================

## Introduction

RWMapMaker is a tool that creates a 2D map of a Train Simulator 2013 Route on 
your computer's monitor. The map can include a representation of the height of 
the land, the roads and railways within the route, and the location (but not 
detailed shape) of many of the houses, trees, factories and other structures 
placed within the route. There are also some not-so-obvious things that can be 
shown, including the boundaries of the tile files that are used by Train 
Simulator 2013 to store the route, the origin, to which all coordinates are 
referenced. You can also read off the status bar the exact location of the 
mouse pointer of any point in the route. In this document, "Train Simulator 
2013" and "TS2013" refer to the game program created by Railsimulator.com. 
RWMapMaker is an independently created addon to that game. All Copyrights and 
Trademarks belong to their respective owners. 

## Why?

RWMapMaker has been created as an aid to route-building using Train 
Simulator 2013:

 - As a way to get an overview of the route, what has been built 
and what is lacking;
 - As a way to quickly locate tiles in the route (to find 
out that "problem in tile 3,7" means a problem at Smith Junction);
 - As a way 
to get maps and plans for inclusion in manuals and web material;
 - And just to see and admire what has been done!

As development has proceeded, some other uses have become apparent:

 - It parses the Train Simulator 2013 files using an 
independently created tools, so it can log issues that Train Simulator 2013 
does not report, making problems easier to locate; 

 - There are opportunities to "mine" the information loaded from the route, for 
example to visually locate objects. While this is not done in the current 
version, it is likely to be in a subsequent version. 

## Requirements 

To run RWMapMaker you will need a computer running Microsoft Windows, as is 
true for TS2013 itself. The program has been tested on Windows 7, both 32-bit 
and 64-bit, and should work on Windows 7, Windows 8 (Desktop mode only) and 
Windows Server 2008. It may work on Windows XP (32-bit) but this is not 
guaranteed or supported. 

## Building

The program can be built using Visual Studio 2010 SP1, and probably with Visual
Studio 2012. You will also need an installation of Qt for Windows 5.2 and
the Qt Visual Studio plugin installed.

Building the Debug variant should create an RWMapMaker executable. The Qt DLLs
need to be available to the program - either in the PATH or in the Debug
directory itself.

Building the Release variant should build the RWMapMaker executable and then
create a Setup MSI file which can install it on a remote computer. 

## Installation

The installation MSI file is a MSI (or Microsoft Installer) file. It 
can be installed by opening it on any current version of Windows. It will 
prompt for an installation location in the normal “wizard” style and place
a program start shortcut in the IvimeyCom sub-folder.

To start RWMapMaker, invoke the program shortcut. When you do this for the 
first time, you can chose whether to run the program in Trial mode, or to
enter the license key you bought with the download.

## Operation

For details of using the program, please refer to the manual included in
the package.

## Issues

There are some known issues:

 - The pogram doesn't currently understand the Zip format of routes. In
 order to see such routes you have to expand the files by hand.
 
 - The maths of transition curves as implemented in Train Simulator has
 so far eluded me, so they are not displayed; in consequence there are some
 gaps in the track, though in many cases surprisingly few.

