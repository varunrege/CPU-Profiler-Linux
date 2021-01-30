## [M1: point 1]
#  Here we define the name of the module.
#  ...
MODULE	 = perftop

## [M2: point 1]
#  Here we create the object file of the module.
#  ...
obj-m += $(MODULE).o

## [M3: point 1]
#  Here we specify the directory of the kernel module.
#  ...
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

## [M4: point 1]
#  Here the PWD variable is used to give the current directory.
#  ...
PWD := $(shell pwd)

## [M5: point 1]
#  Here the 'all' variable is the default target for compiling the module. 
#  ...
all: $(MODULE)


## [M6: point 1]
#  Here we define a rule that applies to all files ending in the .o suffix. The rule says that the .o file depends upon the .c version of the file.
#  The rule then says that to generate the .o file, make needs to compile the .c file using the CC compiler.
#  The -c flag says to generate the object file, the -o $@ says to put the output of the compilation in MODULE and the $< is the first item in the dependencies list.
#  ...
%.o: %.c
	@echo "  CC      $<"
	@$(CC) -c $< -o $@

## [M7: point 1]
#  Explain following in here
#  ...
$(MODULE):
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

## [M8: point 1]
#  Here we include a clean rule for cleaning up source and object directories.
#  ...
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
