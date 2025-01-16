SHELL := /bin/sh
.ONESHELL:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

all: BBCBASIC.EXE

FILES := main.asm ram.asm patch.asm init.asm exec.asm eval.asm interrupts.asm misc.asm sound.asm graphics.asm sorry.asm fpp.asm

.PHONY: BBCBASIC.EXE
BBCBASIC.EXE:
	@mkdir -p ../bin
	@ez80-cc --mem-model 2MB --output-dir ../bin -DEXE=1 -DEZ80CLANG=1 --optimise size BBCBASIC.EXE $(FILES)


