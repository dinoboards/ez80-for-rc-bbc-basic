SHELL := /bin/sh
.ONESHELL:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

all: BBCBASIC.EXE

FILES := main.asm ram.asm exec.asm eval.asm interrupts.asm misc.asm sound.asm graphics.asm sorry.asm fpp.asm cpm.asm spike.c ez80-timer.c v99x8.asm ez80-eval.asm graphics.c osinit.c vdu.c sysfont.asm
FILES += vdu/vdu_08.c vdu/vdu_10.c vdu/vdu_12.c vdu/vdu_13.c vdu/vdu_16.c vdu/vdu_17.c vdu/vdu_18.c vdu/vdu_19.c vdu/vdu_22.c vdu/vdu_23.c vdu/vdu_24.c vdu/vdu_25.c vdu/vdu_28.c vdu/vdu_29.c vdu/vdu_31.c
FILES += vdu/variables.c vdu/logic_to_physical_coords.c

.PHONY: BBCBASIC.EXE
BBCBASIC.EXE:
	@mkdir -p ./bin
	@ez80-cc --lib v99x8-standard --mem-model 2MB --output-dir  ./bin --optimise size BBCBASIC.EXE $(FILES)

.PHONY: format
format: SHELL:=/bin/bash
format:
	@find \( -name "*.c" -o -name "*.h" \) -exec echo "formating {}" \; -exec ez80-clang-format -i {} \;
