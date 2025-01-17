SHELL := /bin/sh
.ONESHELL:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

all: BBCBASIC.EXE

# ; init.asm
FILES := main.asm ram.asm exec.asm eval.asm interrupts.asm misc.asm sound.asm graphics.asm sorry.asm fpp.asm cpm.asm spike.c

.PHONY: BBCBASIC.EXE
BBCBASIC.EXE:
	@mkdir -p ./bin
	@ez80-cc --mem-model 2MB --output-dir ./bin --optimise size BBCBASIC.EXE $(FILES)



