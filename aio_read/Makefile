NAME=aio_read
GLIBC_PACKAGE_VER=$(shell rpm -q glibc | grep -Eo "([0-9\.]+-[0-9\.]+[0-9])")
LIVEPATCHES=librt_$(GLIBC_PACKAGE_VER)_livepatch1.so libc_$(GLIBC_PACKAGE_VER)_livepatch1.so

include ../common/Makefile.common
