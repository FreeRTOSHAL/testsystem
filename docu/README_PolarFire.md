PolarFire SOC
=============
This document descrip all needed infomration to run FreeRTOSHAL on a PolarFire SOC.

Debbuing
--------
Set Jumper (J9) for Embedded Debbuing, debbuing over USB. 
For externel Debbuing unset Jumper (J9), waring incompatible with ARM Debugger!

JTAG Pinout (J23): 
|----|------------|
| 1  | JTAG_TCK   |
| 2  | GND        | 
| 3  | JTAG_TDO   | 
| 4  | NC         | 
| 5  | JTAG_TMS   |
| 6  | VCC        |
| 7  | NC         | 
| 8  | JTAG_nTRST |
| 9  | JTAG_TDI   | 
| 10 | GND        | 
|----|------------|

Debugger Software
-----------------
The Software for the Embedded Debuger is Microchip SoftConsole and can be donloaded at: (Registiation required)

https://www.microsemi.com/product-directory/design-tools/4879-softconsole#downloads

The Debugger Software contains an Eclipse and a modified openocd version. With this OpenOCD Debbging over the Embedded Debugger is possible. 

Run OpenOCD with the folloing command: 

```
sudo ./bin/openocd --command "set DEVICE MPFS" --file board/microsemi-riscv.cfg
```

After this, it is possible to connect an GDB on Port 3333. 

With the GDB `thread` command it is possible to switch between the 5 Harts.
|----------|------------------|
| ThreadID |                  |
|----------|------------------|
| 1        | Monitor Core E51 |
| 2        | U54 Core 1       | 
| 3        | U54 Core 2       | 
| 4        | U54 Core 3       | 
| 5        | U54 Core 4       | 
|----------|------------------|

With the Command `info thread` 
It is possible with the command `info thread` to get the status of all cores. 

.gdbinit for Debugging: 

```
target extended-remote :3333
set mem inaccessible-by-default off
thread 2 # Connect to First U54 Core
```

TFTP Boot
---------

Copy polarfire.dtb in your TFTP directory. This DTB is mateory to boot over TFTP, the bootm commnad need a Device Tree Blob to boot the image. 
This DTB was extracted from the fitImage in the boot patrtion of polarfire board.

Wait until this message is shown on the second UART (ttyUSB1):

```
U-Boot 2020.01 (Oct 09 2020 - 11:52:08 +0000)

DRAM:  1 GiB
error setting mac-address property
MMC:   sdhc@20008000: 0
In:    serial@20100000
Out:   serial@20100000
Err:   serial@20100000
Net:   
Warning: ethernet@20112000 using MAC address from ROM
eth0: ethernet@20112000
Hit any key to stop autoboot: 
```
Hit `Enter` to access U-Boot Console then enter following commands:

U-Boot Script ip via DHCP:

```
env set loadaddr '0x80000000'
env set bootfile 'test.img'
env set fdtfile 'polarfire.dtb'
env set fdtaddr 0x88000000
env set serverip 192.168.1.1
env set netargs 'setenv bootargs console=${console} ${optargs} root=/dev/nfs nfsroot=${serverip}:${rootpath},${nfsopts} rw ip=dhcp'
env set netboot 'echo Booting from network ...; env set autoload no; dhcp; env set loadaddr '0x80000000'; tftp ${loadaddr} ${bootfile}; tftp ${fdtaddr} ${fdtfile}; run netargs; bootm ${loadaddr} - ${fdtaddr}'
env set bootcmd run netboot
```

U-Boot Script with static IP:

```
env set loadaddr '0x80000000'
env set bootfile 'test.img'
env set fdtfile 'polarfire.dtb'
env set fdtaddr 0x88000000
env set serverip 192.168.178.46
env set ipaddr 192.168.178.51
env set netargs 'setenv bootargs console=${console} ${optargs} root=/dev/nfs nfsroot=${serverip}:${rootpath},${nfsopts} rw ip=dhcp'
env set netboot 'echo Booting from network ...; env set autoload no; env set loadaddr '0x80000000'; tftp ${loadaddr} ${bootfile}; tftp ${fdtaddr} ${fdtfile}; run netargs; bootm ${loadaddr} - ${fdtaddr}'
env set bootcmd run netboot
```

After this, Boot the image with the command `boot`

Qemu
----

It is possible to boot under Qemu a detail description can be found in Qemu Source Code under `<qemu soruce>/docs/system/riscv/:microchip-icicle-kit.rst`

```
git clone git://git.qemu-project.org/qemu.git
```

Only a limit of devices is correctly supported by Qemu.
The following devices are Supported by now:

 - Core Level Interruptor (CLINT)
 - Platform Level Interrupt Controller (PLIC)
 - Embedded Non-Volatile Memory (eNVM)
 - Multi-Mode UART (MMUART)
 - Cadence eMMC/SDHC controller and an SD card connected to it
 - SiFive Platform DMA
 - Gigabit Ethernet MAC Controller (GEM)
 - DDR Memory Controller
 - IOSCB module (Clocks, PLLs, ...)

**Precondition:**

Create TAB Interface interface for Ethernet: 
```
sudo tunctl -t tap1 -u `whoami`
sudo ifconfig tap1 up
sudo ifconfig tap1 192.168.1.1 up
```

**Run Qemu**

Run Qemu with the following command:

```
qemu-system-riscv64 -M microchip-icicle-kit -smp 5 \
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
      -serial chardev:serial4
```

The Command create 5 serial Ports for each CPU one Serial Port. The first Serial Port is written to
`stdio` and the other Serial Ports must be connected at run time over telenet with the following command: 

```
telenet localhost 8123 # Serial Port 2
telenet localhost 8124 # Serial Port 3
telenet localhost 8125 # Serial Port 4
telenet localhost 8126 # Serial Port 5
```

GDB can be attached over the `-s` option. GDB port is 1234 the whole processor is connected.
With the Qemu Option `-S` it is possible to stop the processor until GDB is attached.

If the Application is compiled with hardware floating-point support the GDB is attached to the monitor core.
While the symbol loading the GDB detect a mismatch of the floating length and crash with the error:
```
bfd requires flen 4, but target has flen 0
```

A workaround for this is start GDB attach to the Qemu and change to the first U54 Core and then loading the symbol.
This can be automatically achieved with a `.gdbinit` script and execute GDB without a file name:

```
target extended-remote :1234
# add a new cluster
add-inferior
# swtich to the new cluster
inferior 2
# atach to the second cluster
attach 2
set schedule-multiple on
# Swtich to first U54 Core
thread 2.1
add-symbol-file <elf file>
```
