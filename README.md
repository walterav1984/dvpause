# DVPause

Utility for [DVSwitch](http://dvswitch.alioth.debian.org/wiki/) that allows you to pause videos while feeding them to dvsource-file through a pipe.
This also provides you a nice selection of different locally saved movies. It is supposed to be used for events that require live feeding and pausing with all kinds of locally stored videos.
**However**, yet only PAL is tested. NTSC is supported, but you will propably have to change
```C
#define SEQUENCES 12 // PAL
```
to
```C
#define SEQUENCES 10 // NTSC
```

## Dependencies:
GTK+ (Tested with GTK 3.10, propably 3.6+ supported)
dvsource-file (for make run)

## Installation:
Compile it with `make` and you're done!
You can start the program with `./main`. I recommend to edit the Makefile to use `make run` if you're doing development. `make run` will compile DVPause + start piping output to dvsource-file, however, you must change dvsource's port + host.

## Configuration:
The configuration is in the "config" file, this is what all the movies must be added to. Two sample scenes are provided.
