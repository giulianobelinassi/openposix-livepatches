NAME=memcpy
GLIBC_PACKAGE_VER=$(shell rpm -q glibc | grep -Eo "([0-9\.]+-[0-9]+(\.[0-9]+)*)")
LIVEPATCHES=libc_$(GLIBC_PACKAGE_VER)_livepatch1.so
include ../common/Makefile.common

clean:
	rm -rf test *.o *.so *.swp *.dsc *.json *.in
