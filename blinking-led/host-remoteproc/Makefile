#
# PRUSS_REMOTEPROC - pruss driver for kernel Remoteproc infrastructure
#

KDIR ?= /lib/modules/`uname -r`/build
INSTALL_DIR = kernel/drivers/remoteproc
default:
	make -C $(KDIR) M=$(PWD)
clean:
	make -C $(KDIR) M=$(PWD) clean
install:
	make INSTALL_MOD_DIR=$(INSTALL_DIR) -C $(KDIR) M=$(PWD) modules_install
