#61850 WirelessHART Gateway makefile

TARGET := gw
TARGET_DIR := ../bin

########################################################################
#Include dirs
INCLUDE := ../build/include
HARTIP := ../ip
COMMON	:=   ../include

########################################################################
#flags
CFLAGS   := -O2 -Wall -Werror -Wmissing-declarations -Wmissing-prototypes
CINCLUDE := -I. -I$(INCLUDE) -I$(COMMON) 
CLIBDIR  := ../build
CLIBS    := -lm -pthread
DEPFLAGS := -MM

########################################################################
#includes
INCLUDE    = ${CINCLUDE}
FLAGS      = ${CFLAGS}
LIBDIR     = ${CLIBDIR}
LIBS       = ${CLIBS}
LIBINCLUDE = $(LIBDIR)

########################################################################
#Setzen alle zusammen..
CC     	 = gcc
LD       = ld
AR       = ar
ARFLAGS  = rvu
CMPFLAGS = ${FLAGS} ${INCLUDE}
LDFLAGS  = ${LIBS}

########################################################################
CSOURCES = $(wildcard *.c)
OBJECTS  = $(CSOURCES:.c=.o)
DEPS     = $(CSOURCES:.c=.d)
OBJS	 = $(HARTIP)/*.o #$(COMMON)/*.o $(MODEL)/*.o #$(SIMUL)/*.o $(MONITOR)/*.o $(ROBOTHREAD)/*.o 
LIBFILES = $(LIBINCLUDE)/libiec61850.a

########################################################################
#RTAI
#prefix := $(shell rtai-config --prefix)

#ifeq ($(prefix),)
#$(error Please add <rtai-install>/bin to your PATH variable)
#endif

#LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
#LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

########################################################################

#LIBFILES += $(LXRT_LDFLAGS)

#General
QUIET = @

.PHONY: clean clobber 

all: $(TARGET)
	mv $(TARGET) $(TARGET_DIR)

$(TARGET): $(OBJECTS)
	$(QUIET)echo Compiling executable file: $@
	$(QUIET)${CC}  -g -o $@  $^ $(OBJS) $(LIBFILES) $(LXRT_LDFLAGS) $(LDFLAGS)

%.o: %.c
	$(QUIET)echo Creating object: $@
	$(QUIET)$(CC) $(CMPFLAGS) $(LIBS) -c $(LXRT_CFLAGS) $^

clean:
	$(QUIET)echo Removing temp and object files
	$(QUIET)rm -f *~ *.bak *.o $(DEPS) 

clobber: clean
	$(QUIET)echo Removing executable and some unnecessary files
	$(QUIET)rm -f $(TARGET)

-include $(DEPS)
%.d: %.c
	$(QUIET)echo Creating dep: $@
	$(QUIET)$(CC) $(DEPFLAGS) $(LIBS) $(CMPFLAGS) $< \
	| sed 's!\($*\)\.o[ :]*!\1.o $@ : !g' > $@; \
	[ -s $@ ] || rm -f $@

