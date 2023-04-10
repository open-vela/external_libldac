#
#  Copyright (C) 2013 - 2021 Sony Corporation
#


CC		= gcc

CDEBUGFLAGS	= -O3
INC_DIR		= ../inc
SRC_DIR		= ../src
TEST_DIR	= ../test
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

LINKER		= $(CC)
CFLAGS		= $(CCOPTIONS) $(CDEBUGFLAGS) $(INCLUDES) $(DEFINES)
CFLAGS_SH	= $(CFLAGS) $(DEFINES_SH)
LDFLAGS		= $(CCOPTIONS) $(CDEBUGFLAGS)
#LDFLAGS		+= -pthread

LDACDEC		= test_ldacBT_dec

SRCS_LDACDEC	= $(TEST_DIR)/main_ldacBt_o.c

OBJDIR4SH	= ./obj-so
OBJS_LDACDEC	= $(addprefix $(OBJDIR4SH)/, $(notdir $(SRCS_LDACDEC:.c=.o)))

all:$(LDACDEC)

$(OBJDIR4SH)/%.o:$(TEST_DIR)/%.c
	@if [ ! -e $(OBJDIR4SH) ]; then mkdir -p $(OBJDIR4SH); fi
	$(CC) $(CFLAGS_SH) -o $@ -c $<

$(LDACDEC):	$(OBJS_LDACDEC)
	$(LINKER) -o $@ $(LDFLAGS) $(INCLUDES) $^ -lm -ldl


clean:
	rm -rf $(OBJDIR4SH) $(LDACDEC) *.o core core.*

