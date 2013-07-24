#Main Makefile

#defines
SRC 	 = src
CLI		 = cli

DIRS =  $(SRC) $(CLI) 
# DIRS += $(PTOS) $(AT) 

#General
QUIET = @

.PHONY: all clean install distclean clobber doc

all:
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
