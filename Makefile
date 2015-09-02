PROJECT_NAME    := test
objs-y		:= src arch mach driver
libs-y          := freertos
#libs-y          := libs
KBUILD_CLFLAGS := -Wall

include scripts/Makefile.project

