PROJECT_NAME    := test
objs-y		:= src arch mach driver freertos
subdir-y                 := docu/
#libs-y          := libs
KBUILD_CLFLAGS := -Wall

include scripts/Makefile.project

docu: FORCE
	$(Q)$(MAKE) $(build)=docu docu
.PHONY: FORCE
