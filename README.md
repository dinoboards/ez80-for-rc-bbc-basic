# BBC BASIC port for ez80-for-rc kit

A port of BBC Basic for eZ80 to run in ADL mode on the ez80-for-rc RC2014/RCBus kit

### Status

Currently not yet operational - very much a work in progress...

### Derived

This code based is derived from the port of BBCBasic for Z80 to the AGON

### What is BBC Basic for Z80?

The original version of BBC Basic was written by Sophie Wilson at Acorn in 1981 for the BBC Micro range of computers, and was designed to support the UK Computer Literacy Project. R.T.Russell was involved in the specification of BBC Basic, and wrote his own Z80 version that was subsequently ported to a number of Z80 based machines. [I highly recommend reading his account of this on his website for more details](http://www.bbcbasic.co.uk/bbcbasic/history.html).

As an aside, R.T.Russell still supports BBC Basic, and has ported it for a number of modern platforms, including Android, Windows, and SDL, which are [available from his website here](https://www.bbcbasic.co.uk/index.html).

### What is ADL mode?

ADL stands for Address Data Long. When the eZ80 is switched into this mode, the eZ80 runs natively in 24-bit mode.

### Why am I doing this?

To enable the BBC Basic language for the CP/M extended memory model developed for the eZ80-for-rc kit.

### Assembling and Running

TODO: Notes to come soon


### License as per Agon Port by Dean Belfield

This code is distributable under the terms of a zlib license. Read the file [COPYING](COPYING) for more information.

Many thanks to R.T. Russell for open sourcing the source code, and David Given for facilitating this.

http://cowlark.com/2019-06-14-bbcbasic-opensource/index.html

The BASIC interpreter, as originally written by R.T. Russell and [downloaded from David Given's GitHub page](https://github.com/davidgiven/cpmish/tree/master/third_party/bbcbasic), has been modified either for compatibility reasons when assembling using the ZDS IDE, or for development reasons for this release.

The original files are: [eval.z80](eval.z80), [exec.z80](exec.z80), [fpp.z80](fpp.z80), [patch.z80](patch.z80), [main.z80](main.z80), [ram.z80](ram.z80) and [sorry.z80](sorry.z80), [bbcbasic.txt](bbcbasic.txt), the license ([COPYING](COPYING)) and all the files in the examples folder.

Any additions or modifications I've made to port this to the Agon have been released under the same licensing terms as the original code, along with any tools, examples or utilities contained within this project. Code that has been copied or inspired by other sources is clearly marked, with the appropriate accreditations.

### Derivative work

As per Dean Belfield, R.T. Russell and all other contributors
