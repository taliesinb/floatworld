#README

WARNING: *THIS PROJECT IS NOT YET READY FOR PUBLIC CONSUMPTION. CHECK IT OUT AT YOUR OWN RISK, AS IT IS CHANGING VERY FAST*

## About *floatworld*	

*floatworld* is an artificial life environment in which virtual creatures ('Creats') compete for space and energy.

Each creature is controlled by a recurrent neural network and harvests energy in order to reproduce. Time and space are discretized. 

Creats obtain a variety of data from their environments that is fed to their neural network inputs, including a simple short-range 'sense of smell' and information about the state of their bodies.

Random mutation and natural selection result in the evolution of different strategies. Creats can variously learn to co-operate, fight, become efficient at harvesting energy, and perform simple communication.

Unlike [genetic algorithms](http://en.wikipedia.org/wiki/Genetic_algorithm), the form of [evolutionary computation](http://en.wikipedia.org/wiki/Evolutionary_computation) involved in *floatworld* is open-ended: no fitness function is specified, and Creats can use whatever strategies they can to maximize their reproduction and survival.  

Objects can be introduced into the environment to interact with the creatures. 

## Building floatworld

I use Qt Creator as my IDE (it's pretty good, and integrates cleanly with various Qt tools). 

Download and install the Qt SDK for your platform from [their website](http://qt.nokia.com/downloads/downloads).

I've only built *floatworld* on Snow Leopard, so I can't yet speak about building it on other platforms. The build process is painless: first ask Qt to produce a makefile using the `qmake` library, then actually build the sources using `make`. In more detail:

To build an appropriate Makefile using `qmake`, run the following command from the project directory. 

    qmake floatworld.pro -r -spec macx-g++

Then run 

    make

The executable should then appear under `build/floatworld.app/Contents/MacOS`. I'm not providing a packaged executable as of yet.

## Existing functionality

*   Simulation at various speeds
*   Display of floatworlds as simple bitmaps
*   Loading and saving of floatworlds and their contents
*   Simple objects like impassable blocks and [Skinner boxes](http://en.wikipedia.org/wiki/Skinner_box)
*   Changing of various world parameters
*   Inspection of individual Creat activity

## Planned functionality

*   Drag and drop management of objects within the floatworld
*   Animation of Creat movements and interactions using vector graphics
*   Pluggable analysis widgets to monitor population size and dynamics, genetic diversity, and Creat behavior
*   A library of loadable scenarios to illustrate various emergent phenomena
*   A variety of interesting new objects for Creats to interact with
*   Snapshotting of simulation runs so they can be rewound

## Similar projects

A good hub is [Biota.org](http://www.biota.org/), but some (far more impressive) projects that run in a similar vein are:

*   [Tierra](http://life.ou.edu/tierra/)
*   [Avida](http://devolab.msu.edu/)
*   [Polyworld](http://sourceforge.net/projects/polyworld/)
*   [Darwin@Home](http://www.darwinathome.org/)
