#!/bin/sh

make ARCH=riscv CROSS_COMPILE=~/toolchain/riscv-linux/bin/riscv64-unknown-linux-gnu- -j BUILD_TYPE=RELEASE

