# BeatStomper III

A drum sequencer for the Amiga.

Basically this is a 20 year old program simulating a 30 year old drum machine on a 25 year old computer system ;)

This version was planned as a commercial release in 1993, but was never published at that time ([or was it?](http://www.amigafuture.de/asd.php?asd_id=3816)).
It is now available under a BSD style license.

![Screenshot](https://raw.github.com/hwestphal/amiga-beatstomper/master/Screenshot.png)


## Building the programm

BeatStomper III can be built with [vbcc](http://sun.hasenbraten.de/vbcc/) under the cross-development environment provided by the [M68K AmigaOS Toolchain](https://github.com/cahirwpz/m68k-amigaos-toolchain) of Krystian Baclawski.

To simplify the setup a [Vagrantfile](http://www.vagrantup.com/) is provided for setting up the environment:

    $ vagrant up
    $ vagrant ssh
    > cd /vagrant
    > make


## Running the program

For instant fun I've added a minimal Workbench disk under `dist`, which can started using [WinUAE](http://www.winuae.net/)'s harddisk simulation.
You only need the [Kickstart 2.04 ROM](http://www.lemonamiga.com/help/kickstart-rom/kickstart-rom-2-0-4.php) file. The program was never tested with newer AmigaOS versions.

I've included some sound samples which are taken from the BeatStomper V4.0 release back in 1990.


## Former releases still available on the net

* BeatStomper V2.0: [Franz 063](http://www.amigafuture.de/downloads.php?view=detail&df_id=3116)
* BeatStomper V4.0: [Franz 077](http://www.amigafuture.de/downloads.php?view=detail&df_id=3130)
* ~~BeatStomper II V1.0: [KICKPD 325](http://www.back2roots.org/Get/Beat%20Stomper%202%2C1/)~~

There might also exist a BeatStomper II V1.1 release. But the only place where it is mentioned is [here](http://www.amigafuture.de/kb.php?mode=article&k=2977). At least I've still got the source code... 
