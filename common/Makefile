CC=gcc
AR=ar
CFLAGS=-O2 -g -Wall -fPIC -D_PACKAGE_NAME=$(PACKAGE_NAME) -D_VERSION=$(VERSION)
ARCH := $(shell uname -p)

all: libcommon.a

ifeq ($(ARCH),ppc64le)
libcommon.a: common.o common_ppc64le.o
	$(AR) -rcs $@ $^
else
libcommon.a: common.o
	$(AR) -rcs $@ $^
endif

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

%.o: %.S
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	rm -f *.o *.a

check:
	

install:
	
