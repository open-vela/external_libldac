#
#  Copyright (C) 2013 - 2021 Sony Corporation
#

FIXED_POINT	= xTRUE
ENCODE_ONLY	= xTRUE
DECODE_ONLY	= xTRUE

CC		= gcc

CDEBUGFLAGS	= -O3
INC_DIR		= ../inc
SRC_DIR		= ../src
INCLUDES	= -I. -I./$(INC_DIR) -I./$(SRC_DIR)

CCOPTIONS	= -Wall
#CCOPTIONS	+= -Wcast-qual -Wshadow
CCOPTIONS	+= -Wno-long-long
#CCOPTIONS	+= -g
#CCOPTIONS	+= -pg
#CCOPTIONS	+= -ansi
#CCOPTIONS	+= -pedantic
#CCOPTIONS	+= -pedantic-errors
CCOPTIONS	+= -fPIC -fno-merge-constants

ifeq ($(FIXED_POINT), TRUE)
	DEFINES		+= -D_32BIT_FIXED_POINT
endif
ifeq ($(ENCODE_ONLY), TRUE)
	DEFINES		+= -D_ENCODE_ONLY
endif
ifeq ($(DECODE_ONLY), TRUE)
	DEFINES		+= -D_DECODE_ONLY
endif

ifneq ($(FIXED_POINT), TRUE)
	LIBRARIES	= -lm
endif

LINKER	  = $(CC)
CFLAGS    = $(CCOPTIONS) $(CDEBUGFLAGS) $(INCLUDES) $(DEFINES)
LDFLAGS	  = $(CCOPTIONS) $(CDEBUGFLAGS)

LDACLIB	  = ldaclib.a
LDACBTLIB = ldacBT
ifeq ($(ENCODE_ONLY), TRUE)
	LDACBTLIB	= ldacBT_enc
endif
ifeq ($(DECODE_ONLY), TRUE)
	LDACBTLIB	= ldacBT_dec
endif

LDACBT_LIB_VER_MAJOR = $(shell cat ../src/ldacBT_api.c | grep -m 1 'LDACBT_LIB_VER_MAJOR' | grep -o "[0-9]\{1,3\}")
LDACBT_LIB_VER_MINOR = $(shell cat ../src/ldacBT_api.c | grep -m 1 'LDACBT_LIB_VER_MINOR' | grep -o "[0-9]\{1,3\}")
LDACBT_LIB_VER_BRANCH= $(shell cat ../src/ldacBT_api.c | grep -m 1 'LDACBT_LIB_VER_BRANCH' | grep -o "[0-9]\{1,3\}")

LDACBTLIB_LINKERNAME = lib$(LDACBTLIB).so
LDACBTLIB_SONAME	 = $(LDACBTLIB_LINKERNAME).$(LDACBT_LIB_VER_MAJOR)
LDACBTLIB_SHARED	 = $(LDACBTLIB_LINKERNAME).$(LDACBT_LIB_VER_MAJOR).$(LDACBT_LIB_VER_MINOR).$(LDACBT_LIB_VER_BRANCH)

OBJS_LDACLIB	     = $(SRC_DIR)/ldaclib.o
OBJS_LDACBTLIB	     = $(SRC_DIR)/ldacBT.o

all:$(LDACLIB) $(LDACBTLIB)

$(LDACLIB):	$(OBJS_LDACLIB)
	@echo "Loading $@ ..."
	ar cru $@ $(OBJS_LDACLIB)
	@echo "done"

$(LDACBTLIB):	$(OBJS_LDACBTLIB) $(LDACLIB)
	$(LINKER) -fPIC -shared -Wl,--soname=$(LDACBTLIB_SONAME) -o $(LDACBTLIB_SHARED) $(OBJS_LDACBTLIB) $(LDACLIB) -lm
	ln -sf $(LDACBTLIB_SONAME) $(LDACBTLIB_LINKERNAME)
	ln -sf $(LDACBTLIB_SHARED) $(LDACBTLIB_SONAME)
	@echo "Loading $@.a ..."
	ar cru $@.a $(OBJS_LDACBTLIB) $(OBJS_LDACLIB)
	@echo "done"
#for cygwin
	@case "$(shell uname)" in CYGWIN* ) \
		echo "for cygwin";\
		echo "ln -s $(LDACBTLIB_LINKERNAME) $(LDACBTLIB_LINKERNAME:.so=.dll)";\
		ln -s $(LDACBTLIB_LINKERNAME) $(LDACBTLIB_LINKERNAME:.so=.dll) ;; esac

clean:
	rm -f $(OBJS_LDACLIB) $(LDACLIB) $(OBJS_LDACBTLIB) $(LDACBTLIB_SHARED) $(LDACBTLIB_SONAME) $(LDACBTLIB_LINKERNAME) $(LDACBTLIB).a *.o core core.*

