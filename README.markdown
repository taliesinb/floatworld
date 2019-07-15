#README

## About *floatworld*	

<center><img src="https://i.imgur.com/f4d1k6v.png" width="400"/></center>

*floatworld* is an C++ library and associated GUI that serves as a laboratory for a multi-agent RNN-based simulation in which agents compete for space and energy on a two-dimensional grid. I wrote *floatworld*, along with a [short paper](https://www.dropbox.com/s/wp3pjep7ti9onw2/floatworld-project.pdf?dl=1) about it, during my undergraduate degree.

Each agent is controlled by a recurrent neural network and harvests energy in order to reproduce. Time and space are discretized. 

Agents obtain a variety of data from their environments that is fed to their neural network inputs, including a simple convolutional inputs that let them see the grid, and information about the state of their bodies. 

At every time step agents choose from a variety of actions to perform, ranging from movement to interaction with other agents to reproduction (should they possess sufficient energy). There is no objective fitness function.

Random mutation and natural selection result in the evolution of different strategies. Agents can variously learn to co-operate, fight, become efficient at harvesting energy, and perform simple communication with each other.

Unlike [genetic algorithms](http://en.wikipedia.org/wiki/Genetic_algorithm), the form of [evolutionary computation](http://en.wikipedia.org/wiki/Evolutionary_computation) involved in *floatworld* is open-ended: no fitness function is specified, and agents can use whatever strategies they can to maximize their reproduction and survival.  

Objects, such as "Skinner boxes", can be introduced into the environment to interact with the agents. 

Much like in a programming IDE, various parameters of the world can be controlled 'live' using interactive side panels, as well as parameters of the currently selected agent or object.

Grid states can be saved and loaded, and the phylogenetic tree associated with a particular population is recorded efficiently to allow measures of species relatedness to be computed.

### New World Creator

First we have the dialog that creates a new world, specifying what mixture of different energy sources and objects to use, what various global parameters should be, and what the initial agent neural network looks like.

<center><img src="https://i.imgur.com/dydIwqJ.png" width="600"/></center>

### The World

Here is the main environment, which loading, saving, running the simulation at various speeds, adjusting the parameters of the entire world of various agents or objects within it, and so on:

<center><img src="https://i.imgur.com/xyigY98.png" width="700"/></center>

### Colonisation

After several generations of natural selection, agents are adept at colonising new sources of food rapidly:

<center><img src="https://i.imgur.com/hUUD26w.png" width="500"/></center>

### Frogs

This second example demonstrates some curious 'frogs' that have evolved to occupy small circular portions of territory and occasionally 'hop' over each other when they want to reproduce. 

<center><img src="https://i.imgur.com/twwwvJI.png" width="400"/></center>

### Objects

The third example shows one of the many types of object in action. These are [Skinner boxes](http://en.wikipedia.org/wiki/Operant_conditioning_chamber), which reward agents with a burst of energy if they are 'tapped on' a certain number of times. Here agents have rapidly evolved to supplement the meagre supply of energy that I provided by triggering these boxes. Agents must alternate between tapping the box and collecting the reward, so the behavior is not merely a hardwiring between a vision neuron and the tap neuron.  

<center><img src="https://i.imgur.com/wpnq4pV.png" width="500"/></center>

### Emergent behavior

There are many such examples of fascinating behavior that can emerge. My favorite is the evolution of 'peletons' of agents that escape when a feeding spot is exhausted. Each agent only has a visual range of 2 gridcells. However, if a group of evenly spaced agents is travelling in the same direciton, a single agent at the periphery can spot food, turn towards it, and all other agents subsequently turn towards it too, extending the vision range of those in the middle.

### Competition matrices

To evaluate inter-agent fitness we can compute a competition matrix that summarizes all pairwise interactions. To do this we repeatedly instantiate pairs of agents `A` and `B` in a single arena and measure the ratio of the nubmer agents of the two genotypes `N_A` and `N_B` after a fixed number of simulation timesteps have elapsed. Using `arctan(ln(N_a / N_b))` we can obtain a score in the range `(-1, 1)` to characterize the relative fitness of these two agents.

Here is an example comeptition matrix that represents the pairwise fitness of 20 agents, where each agent is successive sampled at random from the population every 5000 timesteps. Curiously, we can see that natural selection briefly "ran backwards", at least in terms of the pairwise fitness of agents and their descendents. 

<center><img src="https://i.imgur.com/MP8mDv0.png" width="300"/></center>

## Paper on floatworld 

You can find a short paper on floatworld [here](https://www.dropbox.com/s/wp3pjep7ti9onw2/floatworld-project.pdf?dl=1).

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
*   Inspection of individual agent activity
*   Animation of agent movements and interactions using vector graphics
*   Snapshotting of simulation runs so they can be rewound

## Planned functionality

*   Drag and drop management of objects within the floatworld
*   Pluggable analysis widgets to monitor population size and dynamics, genetic diversity, and agent behavior
*   A library of loadable scenarios to illustrate various emergent phenomena
*   A variety of interesting new objects for agents to interact with

## Future Ideas

#### Space 

Introduce "symmetry breaking": forget about turn-left/turn-right/move forward. Instead, creature specifies the direction it wants to move in absolute terms. This introduces a whole set of potential feedbacks: a population can have a 'preferred direction', which it might behoove another population to exploit or copy. Arms races, feedback, and so on.

#### Vision

Likewise, forget about kernel-based vision. Instead, introduce a fixed number of cells, relative to the current cell, that a creature can 'sample'. Perhaps these are specified as x/y offsets as part of the genome. Likewise, perhaps allow the creature to "jump" as far as it wants, with the only limitation being a distance-related energy cost. This allows for much richer potential scavenging behavior, escape, and so on.

#### Energy
Also, perhaps do away with floating point energy values? The advantage would be that each cell can have an 'information representation' that is just a bit field. Say, 1 bit for energy content (full/empty), 2+ bits for creature content (which will give creatures scope to try to recognise and identify one another, eventually), perhaps a few 'pheromone' bits that can be set and or queried. 

Another interesting possibility is to have more than one food type. Perhaps creatures can specialize (the most straightforward way would be a hard-coded preference in their genome).

#### Brains
How will this relate to sensor neurons? Do sensor neurons have a mask that determines what will activate them? What if, instead of passive sensor neurons, there is 'active' observation: two of the NN outputs determine 'where to look', and the result is fed back in at the next step? 

This biases the whole information processing structure away from continuous neural networks. 

#### Brain coding

Try to preserve the basic idea of neural net? 

Example: Brain has 32 neurons, brain state is a 32-bit int. Each bit specifies whether given neuron is firing/not firing. Each neuron has 'inhibit mask' and 'excite mask' that is applied to brain state. Count bits to see which wins. This is the same as saying: are more of these following bits on than these other bits? What about a different threshold? This can easily be achieved by having, say, 4 bits that are always on, which can give a way of biasing up or down. 

So, sources of input: Say 8 bits from the sensor cell, a few bits of internal state.

Desired outputs: bit-count to get number from 0 to 8 for each of the X/Y directions. Or instead of having coupled X and Y, what about limiting to either X axis or Y axis? 6 bits to specify 0-6, another 2 bits to specify {N,E,S,W}.
Times this by two, one to specify where to look, the other where to jump. Or perhaps jump and look share a bus, and a single bit determines which to use. 

This seems more and more like a continuous NN. 

Side thought: might want to iterate the brain a couple of rounds before 'making a decision'. 

Also: Could incorporate noisiness by using 'fuzzy logic' type stuff, i.e. bit masks that are randomly resolved at each use. Perhaps also incorporate noise from the external world as well (say, dependent on sensor distance). 

### Complexity

There is a natural analogy to complexity here, in that a "complex" organism is deemed to be one that employs more than one active sensor at a time. To allow for this, brain topology would have to be dynamic, some kind of generative system. Then, natural questions are: is there a drive towards greater number of sensors? Under what conditions?

## Similar projects

A good hub is [Biota.org](http://www.biota.org/), but some projects that run in a similar vein are:

*   [Tierra](http://life.ou.edu/tierra/)
*   [Avida](http://devolab.msu.edu/)
*   [Polyworld](http://sourceforge.net/projects/polyworld/)
*   [Darwin@Home](http://www.darwinathome.org/)
