DEVICE=$(notdir $(CURDIR))
export DEVICE

all:
	@$(MAKE) -C .. $(MAKEFLAGS)

%: force
	@$(MAKE) -C .. $@ $(MAKEFLAGS)

force: ;
