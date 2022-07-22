VERSION=0.5

DIRS = $(sort $(dir $(wildcard */)))
LIST = $(DIRS:/=)

CLEAN_LIST = $(foreach word,$(LIST),clean-$(word))
CHECK_LIST = $(foreach word,$(LIST),check-$(word))
INSTALL_LIST = $(foreach word,$(LIST),install-$(word))

.PHONY:$(LIST)
.PHONY:$(CLEAN_LIST)
.PHONY:$(CHECK_LIST)
.PHONY:$(INSTALLLIST)
.PHONY:clean
.PHONY:dist

all: download $(LIST)

# Download libs
download:
	$(MAKE) -C glibc download

$(LIST): download
	$(MAKE) -C $@

$(CLEAN_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) clean

$(CHECK_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) check > /dev/null

$(INSTALL_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) install

clean: $(CLEAN_LIST)
	rm -f openposix-livepatches-$(VERSION).tar.xz

check: $(CHECK_LIST)

install: $(INSTALL_LIST)

dist: clean
	tar cJf openposix-livepatches-$(VERSION).tar.xz --exclude=glibc \
	--exclude=openposix-livepatches-$(VERSION).tar.xz *
