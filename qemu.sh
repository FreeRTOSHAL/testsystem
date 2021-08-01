#!/bin/bash
. include/config/auto.conf
QEMU=
ARGS=$*
if [ "$CONFIG_MACH_RISCV_QEMU" == "y" ]; then
	if [ "$CONFIG_ARCH_64BIT" == "y" ]; then
		QEMU=qemu-system-riscv64
		if [ "$CONFIG_ARCH_RISCV_MMODE" == "y" ]; then
			QEMUARG="-nographic -machine virt -net none -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline -bios none -smp 4 -kernel test"
		fi 
		if [ "$CONFIG_ARCH_RISCV_SMODE" == "y" ]; then
			QEMUARG="-nographic -machine virt -net none -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline -smp 4 -kernel test"
		fi
	else
		QEMU=qemu-system-riscv32
		if [ "$CONFIG_ARCH_RISCV_MMODE" == "y" ]; then
			QEMUARG="-nographic -machine virt -net none -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline -bios none -smp 4 -kernel test"
		fi 
		if [ "$CONFIG_ARCH_RISCV_SMODE" == "y" ]; then
			QEMUARG="-nographic -machine virt -net none -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline -smp 4 -kernel test"
		fi
	fi
fi
if [ "$CONFIG_MACH_POLARFIRE" == "y" ]; then
	QEMU=qemu-system-riscv64
	QEMUARG="$ARGS -M microchip-icicle-kit -smp 5 \
      -bios /home/andreas/boards/riscv/microchip_hart-software-services/Default/hss.bin -sd core-image-minimal-dev-icicle-kit-es-sd-20201009141623.rootfs.wic \
      -nic user,model=cadence_gem \
      -nic tap,ifname=tap1,model=cadence_gem,script=no \
      -display none -serial stdio \
      -chardev socket,id=serial1,host=localhost,port=8123,server=on,wait=on \
      -serial chardev:serial1 \
      -chardev socket,id=serial2,host=localhost,port=8124,server=on,wait=on \
      -serial chardev:serial2 \
      -chardev socket,id=serial3,host=localhost,port=8125,server=on,wait=on \
      -serial chardev:serial3 \
      -chardev socket,id=serial4,host=localhost,port=8126,server=on,wait=on \
      -serial chardev:serial4"
fi
if [ -z "$QEMU" ]; then
	echo "No Emulation possible"
	exit 1
fi
echo $QEMU $ARGS $QEMUARG
$QEMU $ARGS $QEMUARG
