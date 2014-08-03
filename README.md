UltraEdit
=========

Manager for Ultrastar Songs. This is still in very early stages
and does not modify your song files at the moment (to prevent any cat-eating)

But it can already load songs, detect errors in them, play them and edit the notes.

Requirements
============

The project is developed using Qt 5.3
It may work with earlier versions but was not tested. 
It will definitely not work with Qt4.

The project further requires the use of drumstick for midi output. As song editing is an integral part of the program this is *not* an optional dependency.
It is possible that you have to build the current version of drumstick yourself to receive a Qt5 compatible version.

Drumstick can be found here:

http://drumstick.sourceforge.net/

I further recommend QSynth as a frontend for the FLUID Synth software synthesizer. This will allow you to hear the midi notes using a soundfont.
There are many tutorials on how to setup QSynth. But if you have questions: ask away. You can select the midi output channel of UltraEdit in the settings menu.

*Gentoo*
A gentoo ebuild for svn head of drumstick is included in the dist/gentoo/ directory and can be used to build a local version.
Beware that this is not slotted and *will* break other programs that require a Qt4 version of drumstick! Do not use the ebuild unless you have no other drumstick dependent programs installed that you use!

Building
========

cd build/

qmake ../

make

That should leave you with a working binary. Have fun :)
