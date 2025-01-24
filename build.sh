#!/usr/bin/env bash

set -e

echo "Compiling spike.c -> spike.s"
ez80-clang ${INCLUDE_PATHS} -nostdinc -ffunction-sections -fdata-sections -Oz -Wall -Wextra -Wunreachable-code -Werror -mllvm -z80-print-zero-offset -S spike.c -c -o spike.s

echo "Compiling ez80-timer.c -> ez80-timer.s"
ez80-clang ${INCLUDE_PATHS} -nostdinc -ffunction-sections -fdata-sections -Oz -Wall -Wextra -Wunreachable-code -Werror -mllvm -z80-print-zero-offset -S ez80-timer.c -c -o ez80-timer.s

echo "Compiling graphics.c -> graphics.s"
ez80-clang ${INCLUDE_PATHS} -nostdinc -ffunction-sections -fdata-sections -Oz -Wall -Wextra -Wunreachable-code -Werror -mllvm -z80-print-zero-offset -S graphics.c -c -o graphics.s

echo "Compiling vdu.c -> vdu.s"
ez80-clang ${INCLUDE_PATHS} -nostdinc -ffunction-sections -fdata-sections -Oz -Wall -Wextra -Wunreachable-code -Werror -mllvm -z80-print-zero-offset -S vdu.c -c -o vdu.s

# echo "Compiling hello-world.s -> hello-world.o"
# ez80-as -march=ez80+full -a=./hello-world.lst ./hello-world.s -o ./hello-world.o

# ez80-ld -T /opt/ez80-clang/linker-scripts/cpm-2mb.ld --relax -O1 --strip-discarded -Map=./hladl.map --orphan-handling=error -L/opt/ez80-clang/lib/ --whole-archive -lcrtexe --no-whole-archive -lcrt hello-world.o  --start-group -llibc  -lcrt -lcpm -lv99x8-hdmi -lez80 -lmalloc -lio -lez80rc --end-group --oformat binary -o ../bin/HWADL.EXE
