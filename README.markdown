#README

#### WARNING

*THIS PROJECT IS NOT READY FOR PUBLIC CONSUMPTION. IT HAS BEEN PORTED FROM THE ALLEGRO GRAPHICS LIBRARY TO QT AND MANY FEATURES REMAIN TO BE IMPLEMENTED. IT CONTAINS NOTHING INTERESTING FOR NOW. DON'T CHECK IT OUT.*

#### About floatworld

*floatworld* is an artificial life environment in which virtual creatures compete for space and energy.

Each creature is controlled by a neural network and harvests energy in order to reproduce.

Random mutation and natural selection result in the evolution of different strategies.

Objects can be introduced into the environment to interact with the creatures.

This project is build with Qt Creator. To build an appropriate Makefile, run the following command from the project directory:

    qmake floatworld.pro -r -spec macx-g++

Then run Make.

The executable should appear in a subdirectory of build directory.