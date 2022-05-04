DIRS = $(sort $(dir $(wildcard */)))
LIST = $(DIRS:/=)

CLEAN_LIST = $(foreach word,$(LIST),clean-$(word))
CHECK_LIST = $(foreach word,$(LIST),check-$(word))

.PHONY:$(LIST)
.PHONY:$(CLEAN_LIST)
.PHONY:$(CHECK_LIST)
.PHONY:clean

all: $(LIST)

$(LIST):
	$(MAKE) -C $@

$(CLEAN_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) clean

$(CHECK_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) check

clean: $(CLEAN_LIST)

check: $(CHECK_LIST)
