DIRS = $(sort $(dir $(wildcard */)))
LIST = $(DIRS:/=)

CLEAN_LIST = $(foreach word,$(LIST),clean-$(word))

.PHONY:$(LIST)
.PHONY:$(CLEAN_LIST)
.PHONY:clean

all: $(LIST)

$(LIST):
	$(MAKE) -C $@

$(CLEAN_LIST):
	$(MAKE) -C $(word 2,$(subst -, ,$@)) clean

clean: $(CLEAN_LIST)
