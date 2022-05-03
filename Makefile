LIST=access

.PHONY:$(LIST)

all: $(LIST)

access:
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(MAKE) -C access clean
