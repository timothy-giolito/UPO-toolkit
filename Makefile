docpath=./doc/api

CFLAGS+=-Wall -Wextra -std=c11 -pedantic -g -I"$(PWD)/include" -fPIE

export CFLAGS

.PHONY: all apps bin clean doc test

all: bin test apps

bin:
	cd bin && $(MAKE) static

apps: bin
	cd apps && $(MAKE) all

test: bin
	cd test && $(MAKE) all

doc:
	mkdir -p $(docpath)
	doxygen

doc-clean:
	$(RM) -r $(docpath)

clean:
	cd bin && $(MAKE) clean
	cd apps && $(MAKE) clean
	cd test && $(MAKE) clean

realclean: clean doc-clean
