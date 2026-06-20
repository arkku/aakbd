DEVICE=$(notdir $(CURDIR))
export DEVICE

all:
	@$(MAKE) -C ..

%: force
	@$(MAKE) -C .. $@

force: ;
