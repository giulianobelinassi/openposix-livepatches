VERSION=0.18
PACKAGE_NAME=openposix-livepatches

# List of directories containing tests
LIST = \
aio_read \
aio_return \
common \
free \
malloc \
memcpy \
nanosleep \
pthread_mutex_lock \
pthread_mutex_unlock \
strchr \
strcpy \
strncpy \

CLEAN_LIST = $(foreach word,$(LIST),clean-$(word))
CHECK_LIST = $(foreach word,$(LIST),check-$(word))
INSTALL_LIST = $(foreach word,$(LIST),install-$(word))

.PHONY:$(LIST)
.PHONY:$(CLEAN_LIST)
.PHONY:$(CHECK_LIST)
.PHONY:$(INSTALL_LIST)
.PHONY:clean
.PHONY:dist

all: $(LIST)

$(LIST):
	$(MAKE) VERSION=$(VERSION) PACKAGE_NAME=$(PACKAGE_NAME) -C $@

$(CLEAN_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) clean

$(CHECK_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) check

$(INSTALL_LIST):
	$(MAKE) VERSION=$(VERSION) PACKAGE_NAME=$(PACKAGE_NAME) \
		-C $(word 2,$(subst -, ,$@)) install

clean: $(CLEAN_LIST)
	rm -f openposix-livepatches-$(VERSION).tar.xz

check: $(CHECK_LIST)

install: $(INSTALL_LIST)

dist: clean
	tar cJf openposix-livepatches-$(VERSION).tar.xz --exclude=openposix-livepatches-$(VERSION).tar.xz *
