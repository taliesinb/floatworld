#README

## About *floatworld*	

*floatworld* is an artificial life laboratory in which virtual creatures ('Creats') compete for space and energy. 

Each creature is controlled by a recurrent neural network and harvests energy in order to reproduce. Time and space are discretized. 

Creats obtain a variety of data from their environments that is fed to their neural network inputs, including a simple short-range 'sense of smell' and information about the state of their bodies. 

At every time step Creats choose from a variety of actions to perform, ranging from movement to interaction with other Creats to reproduction (should they possess sufficient energy). There is no objective fitness function.

Random mutation and natural selection result in the evolution of different strategies. Creats can variously learn to co-operate, fight, become efficient at harvesting energy, and perform simple communication.

Unlike [genetic algorithms](http://en.wikipedia.org/wiki/Genetic_algorithm), the form of [evolutionary computation](http://en.wikipedia.org/wiki/Evolutionary_computation) involved in *floatworld* is open-ended: no fitness function is specified, and Creats can use whatever strategies they can to maximize their reproduction and survival.  

Objects can be introduced into the environment to interact with the creatures. 

Various parameters of the world can be controlled 'live' using side panels, as well as parameters of the currently selected Creat or object. 

## Screenshot

For some screenshots, see the [wiki](https://github.com/taliesinb/floatworld/wiki).

It's easier to encourage you to play around for yourself with the code than to explain what everything is in detail -- although I need to write a comprehensive tutorial before that is realistic. 

## Building floatworld

I use Qt Creator as my IDE (it's pretty good, and integrates cleanly with various Qt tools). 

Download and install the Qt SDK for your platform from [their website](http://qt.nokia.com/downloads/downloads).

I've only built *floatworld* on Snow Leopard, so I can't yet speak about building it on other platforms. The build process is painless: first ask Qt to produce a makefile using `qmake`, then actually build the sources using `make`. In more detail:

To build an appropriate Makefile using `qmake`, run the following command from the project directory. 

    qmake floatworld.pro -r -spec macx-g++

Then run 

    make

The executable should then appear under `build/floatworld.app/Contents/MacOS`. I'm not providing a packaged executable as of yet.

### Building floatworld (Ubuntu 10.10 MM)

Ubuntu does not include the most recent Qt version by default in the package system, meaning you'll need to build it if you don't already have it installed.

#### Qt install from source on Ubuntu:

1. Install Qt-sdk from package manager (this should install Qt dependencies):
`sudo apt-get install qt-sdk qt4-qmake`.
2. Download and extract Qt source: [Qt-4.7.0rc1 from Nokia](http://get.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.7.0-rc1.tar.gz)
3. In the qt-everywhere-opensource-src-4.7.0-rc1 directory, compile Qt (note that this will take ~6G of storage space when complete ):
Run `sudo ./configure` and `sudo make`.
4. Once the source is compiled, install Qt using `sudo make install`.

#### floatworld install from source on Ubuntu:
1. Go to the floatworld/ directory and run `qmake floatworld.pro -r`
2. Run `make` to build floatworld in build/ .

## Existing functionality

*   Simulation at various speeds
*   Display of floatworlds as simple bitmaps
*   Loading and saving of floatworlds and their contents
*   Simple objects like impassable blocks and [Skinner boxes](http://en.wikipedia.org/wiki/Skinner_box)
*   Changing of various world parameters
*   Inspection of individual Creat activity
*   Animation of Creat movements and interactions using vector graphics
*   Snapshotting of simulation runs so they can be rewound

## Planned functionality

*   Drag and drop management of objects within the floatworld
*   Pluggable analysis widgets to monitor population size and dynamics, genetic diversity, and Creat behavior
*   A library of loadable scenarios to illustrate various emergent phenomena
*   A variety of interesting new objects for Creats to interact with

## Similar projects

A good hub is [Biota.org](http://www.biota.org/), but some (far more impressive) projects that run in a similar vein are:

*   [Tierra](http://life.ou.edu/tierra/)
*   [Avida](http://devolab.msu.edu/)
*   [Polyworld](http://sourceforge.net/projects/polyworld/)
*   [Darwin@Home](http://www.darwinathome.org/)
