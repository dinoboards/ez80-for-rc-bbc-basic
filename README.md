# BBC BASIC port for ez80-for-rc kit

A port of BBC Basic for eZ80 to run in ADL mode on the ez80-for-rc RC2014/RCBus kit

### Status

Archived.

I have started work on a new version, based on the v5 of BBC Basic.

See https://github.com/dinoboards/bbcbasic-eZ80-for-rc/tree/dean/converting-to-adl

### Custom STAR (*) commands

This BBCBasic interpreter supports a number of platform specific commands.  For the standard Z80 commands see section `5. OPERATING SYSTEM INTERFACE` of the [BBCBasic.txt](/bbcbasic.txt) manual

Additional commands have been added for this specific variant (CP/M ADL) version for the eZ80 for RC

| Command | Description |
|---------|-------------|
| *ASM | parse/assemble ez80 assembly |
| *VERSION | reports the version string of the interpreter |
| *VDP_... | a number of low-level commands to access a V99x8/TMS9918 VDP hardware directly |

There are a number of *VDP_... commands that assume a V99x8 VDP is installed.  It also assumes the standard MSX i/o port numbers are to be used by the ez80 to access the hardware.

The commands map the V99x8 onchip instructions/commands.  For more detail of what the specific
commands do and their arguments, please see the V9938/V9958 manuals.

#### *VDP_CLEAR_MEM

`*VDP_CLEAR_MEM`

This command will zero out all RAM within the VDP.

#### *VDP_CMD_LINE

`*VDP_CMD_LINE x, y, long_length, short_length, direction, colour, operation`

The LINE command draws a straight line in the Video or Expansion RAM. The line drawn is the hypotenuse
that results after the long and short sides of a triangle are defined. The two sides are defined as distances from a single point.

* `x` the starting x-coordinate of the rectangle
* `y` the starting y-coordinate of the rectangle
* `long_length` the number of pixels on the long side
* `short_length` the number of pixels on the short side
* `colour` the colour code to be painted
* `direction` the direction of the painting (DIX_RIGHT, DIX_LEFT, DIY_DOWN, DIY_UP)
* `operation` the logical operation to be performed (CMD_LOGIC_IMP, CMD_LOGIC_AND, ...)

#### VDP_GRAPHIC_MODE

`*VDP_GRAPHIC_MODE mode`

Configure the VDP's control registers to enable the specific graphic mode.

* `mode` must evaluate to a number from 1 to 7 (inclusive)

#### VDP_CMD_VDP_TO_VRAM

`*VDP_CMD_VDP_TO_VRAM x, y, width, height, colour, direction`

VDP command 'High-speed move VDP to VRAM'

This command is used to paint in a specified rectangular area of the VRAM or the expansion RAM.

Since the data to be transferred is done in units of one byte, there is a limitation due to the display mode, on the value for x.

> note that in the G4 and G6 modes, the lower one bit, and in the G5 mode, the lower two bits of x and width, are lost.

* `x` the starting x-coordinate of the rectangle
* `y` the starting y-coordinate of the rectangle
* `width` the width of the rectangle in pixels
* `height` the height of the rectangle in pixels
* `colour` the colour code to be painted (as per the current graphics mode)
* `direction` the direction of the painting (DIX_RIGHT, DIX_LEFT, DIY_DOWN, DIY_UP)

#### VDP_REGWR

`*VDP_REGWR reg_num, reg_val`

Write an 8 bit value to one of the VDP's control registers.

* `reg_num` must evaluate to a valid control register number of the VDP
* `reg_val` must evaluate to an 8 bit number (0-255)

#### *VDP_STATUS

`*VDP_STATUS reg_num, var_name`

Retrieve the current byte from one of the VDP's status registers.

* `reg_num` must evaluate to a valid status register number of the VDP
* `var_name` must be a number type variable name to receive the status register's value

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
