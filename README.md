FreeROTS HAL Testsystem
================================

FreeRTOS HAL Testsystem

FreeRTOS, FreeRTOS.org and the FreeRTOS logo are trademarks of Real Time Engineers Ltd. 

This is an unofficial Repo of FreeRTOS. This part of the FreeRTOS HAL develops at UaS RheinMain. 

Needed Buildtools
-----------------

```sh
sudo apt-get install gcc gdb make autotools-dev automake libtool git gcc-arm-none-eabi libnewlib-arm-none-eabi libncurses5-dev
```

Build
-----
Clone this repo with git clone --recursive or init submoudle with 'git submodule init; git submodule update;'

Configure Project: 

```sh
make tlm_vf610_test_defconfig
```

Build Project: 
```sh
make
```
