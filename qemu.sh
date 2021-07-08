#!/bin/bash
#qemu-system-riscv32 $* -nographic -machine virt -net none -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline -bios none -smp 4 -kernel test
qemu-system-riscv32 $* -nographic -machine virt -net none -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline -smp 4 -bios ~/boards/riscv/opensbi/install/share/opensbi/ilp32/generic/firmware/fw_dynamic.elf -kernel test
