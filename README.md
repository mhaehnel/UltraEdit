UltraEdit
=========

Manager for Ultrastar Songs. This is still in very early stages and may eat any files that you add to a collection (and others as well ;) )
It can already load songs, detect errors in them, play them and edit the notes and edit many aspects of the file.

Requirements
============

The project is developed using Qt 5.6
It may work with earlier versions but was not tested. 
It will definitely not work with Qt4.

The project further requires the use of drumstick for midi output.
As song editing is an integral part of the program this is *not* an optional dependency.
Drumstick can be found here:

http://drumstick.sourceforge.net/

And should be present in most Linux distributions. Version 1.0.0 or newer is
required to use it with Qt5 applications. This is the standard version
for openSUSE, Fedora, Arch (extra) and Gentoo. For Debian and Ubuntu the
packages are (as usual) a bit dated. Please build them yourself there.

I further recommend QSynth as a frontend for the FLUID Synth software synthesizer. This will allow you to hear the midi notes using a soundfont.
There are many tutorials on how to setup QSynth. But if you have questions: ask away. You can select the midi output channel of UltraEdit in the settings menu.

Building
========

cd build/

qmake ../

make

That should leave you with a working binary. Have fun :)
