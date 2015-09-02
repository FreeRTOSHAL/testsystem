PROJECT_NAME    := test
objs-y		:= src arch mach board driver freertos 
#libs-y          := libs
KBUILD_CLFLAGS := -Wall

include scripts/Makefile.project

