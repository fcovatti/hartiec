#Main Makefile

#defines
HARTIP 	 = ip
HARTCLI	 = cli
GATEWAY = gw

DIRS =  $(HARTIP) $(HARTCLI) $(GATEWAY)
# DIRS += $(PTOS) $(AT) 

#General
QUIET = @

.PHONY: all clean install distclean clobber doc

all:
	$(QUIET)cp ../libiec61850/build/libiec61850.a build
	$(QUIET)find ../libiec61850/ -name *.h -exec cp '{}' build/include \;
	$(QUIET)for i in $(DIRS); do cd $$i; $(MAKE); cd -;done
	$(QUIET)find . -name "*.gch" | xargs rm

clean:
	$(QUIET)for i in $(DIRS); do cd $$i; $(MAKE) $@; cd -;  done

install:

clobber: clean
	$(QUIET)for i in $(DIRS); do cd $$i; $(MAKE) $@ ; cd -; done

doc:
	$(QUIET)doxygen Doxyfile

# DO NOT DELETE
