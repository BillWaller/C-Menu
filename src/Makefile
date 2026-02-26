# @file Makefile
# @brief Makefile for C-Menu application suite
# @author Bill Waller
# Copyright (c) 2025
# MIT License
# billxwaller@gmail.com
# @date 2026-02-09
#
# Currently, the $(LIB_DIR) is compiled into the binaries as the runtime
# library path so they can find libcm.so at runtime. That works well for 
# development, but it is not ideal. The best option is probably to place
# libcm.so in a standard location like /usr/lib64.
#
# When you are ready to distribute C-Menu menu, form, pick, view, and other 
# binaries to users, you will probably want them in /usr/local/bin or 
# /usr/bin/bash. However, you should not put rsh in a public directrory,
# at least not until it has secure white-list authentication.
#
# WARNING: Will Robinson
# 
# Before using this makefile, please EDIT THE FOLLOWING VARIABLES to match your 
# system configuration. The makefile assumes that you have a user named "bill" 
# and a group named "bill". If your system uses different names, please update 
# the USER and GROUP variables accordingly. Additionally, ensure that the HOME 
# variable points to the correct home directory for the specified user.
#
# Edit the three lines below to match your system configuration:
USER=bill
GROUP=bill
HOME=/home/$(USER)
PREFIX=$(HOME)/menuapp

# Uncomment the following line to enable static linking of rsh
# RSH_LD = -static
# or export RSH_LD in your environment before running cmake
# export RSH_LD="-static"

# Do not edit below this line unless you know what you are doing.
# The variables above should be sufficient for most users to build and
# install the application suite. If you need to make changes to the build 
# process, please ensure that you understand the implications of those changes 
# before proceeding.
#
# if you intstall the library, libcm.so in other than $(LIB_DIR), you must 
# update the runtime library search path in the binaries as well, or the
# applications may fail to find the shared library at runtime.
#
# ldconfig $(LIB_DIR)

SYSDATE := $(date -u +"%Y-%m-%dT%H:%M:%SZ")

REL_CFLAGS=-O2
DBG_CFLAGS=-O0 -g3
CC=gcc -Wall -Wextra -Iinclude $(DBG_CFLAGS)

MAJOR=0
MINOR=2
PATCH=9
VERSION=$(MAJOR).$(MINOR).$(PATCH)

LIB_NAME=libcm.so
SONAME=$(LIB_NAME).$(MAJOR)
REAL_NAME=$(LIB_NAME).$(VERSION)

LIB_DIR=$(PREFIX)/lib64
BIN_DIR=$(PREFIX)/bin
INC_DIR=$(PREFIX)
LIB_PATH=.:$(LIB_DIR)

BINEXES=menu form pick ckeys view whence rsh iloan enterchr \
		  enterstr lf optsp stripansi

LIB_SRCS=dwin.c futil.c scriou.c exec.c sig.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

COMMON_HDRS=include/cm.h include/common.h include/menu.h \
		include/form.h include/pick.h include/view.h include/version.h
COMMON_SRCS=curskeys.c fields.c form_engine.c init.c init_view.c \
		mem.c mview.c opts.c pick_engine.c view_engine.c
COMMON_OBJS=$(COMMON_SRCS:.c=.o)

MENU_SRCS=menu.c menu_engine.c parse_menu_desc.c
MENU_OBJS=$(MENU_SRCS:.c=.o) $(COMMON_OBJS)

FORM_SRCS=form.c
FORM_OBJS=form.o $(COMMON_OBJS)

PICK_SRCS=pick.c
PICK_OBJS=pick.o $(COMMON_OBJS)

VIEW_SRCS=view.c
VIEW_OBJS=view.o $(COMMON_OBJS)

CKEYS_SRCS=ckeys.c
CKEYS_OBJS=ckeys.o $(COMMON_OBJS)

LIBS = -L. -lcm -lncursesw -ltinfo -lm -Wl,-rpath,$(LIB_PATH)

all:	$(LIB_NAME) $(BINEXES) CMenu.conf
	@echo Make Complete

menu:	$(COMMON_HDRS) $(MENU_OBJS) $(LIB_NAME)
	$(CC) 	$(MENU_OBJS) -o $@ $(LIBS)

form:	$(COMMON_HDRS) $(FORM_OBJS) $(LIB_NAME)
	$(CC) 	$(FORM_OBJS) -o $@ $(LIBS)

pick:	$(COMMON_HDRS) $(PICK_OBJS) $(LIB_NAME)
	$(CC) 	$(PICK_OBJS) -o $@ $(LIBS)

view:	$(COMMON_HDRS) $(VIEW_OBJS) $(LIB_NAME)
	$(CC) 	$(VIEW_OBJS) -o $@ $(LIBS)

ckeys:	$(COMMON_HDRS) $(CKEYS_OBJS) $(LIB_NAME)
	$(CC) 	$(CKEYS_OBJS) -o $@ $(LIBS)

enterchr:	include/cm.h enterchr.o $(LIB_NAME)
	$(CC) 	enterchr.o -o $@ $(LIBS)

optsp:	include/cm.h optsp.o opts.o $(LIB_NAME)
	$(CC) 	optsp.o opts.o -o $@ $(LIBS)

enterstr:	include/cm.h enterstr.o $(LIB_NAME)
	$(CC) 	enterstr.o -o $@ $(LIBS)

whence:	include/cm.h whence.o $(LIB_NAME)
	$(CC) 	whence.o -o $@ $(LIBS)

iloan:	iloan.o $(LIB_NAME)
	$(CC) 	iloan.o -o $@ $(LIBS)

rsh:	include/cm.h rsh.o
	$(CC) $(RSH_LD) rsh.o -o $@

lf: include/cm.h	lf.o $(LIB_NAME)
	$(CC) 	lf.o -o $@ $(LIBS)

stripansi:	include/cm.h stripansi.o $(LIB_NAME)
	$(CC) 	stripansi.o -o $@ $(LIBS)

CMenu.conf:
	echo $(LIB_DIR) >CMenu.conf

$(LIB_NAME): $(LIB_OBJS)
	$(CC) 	-shared -o $@ $^

%.o:	%.c
	$(CC) 	-c $< -fPIC	-o $@

clean:
	rm -rf core *.o $(BINEXES) make.out $(CFLOW_OUTPUT) *.o libcm.so \
		*.cflow scan-build scan-build.out $(LIB_NAME) CMenu.conf \
		x xx xxx *.out .user_id .group_id manifest.txt manifest.ls

test: snippets/strings_test1.c
	$(CC) -o test_strings snippets/strings_test1.c $(LIBS)

install:
	./inst.sh --
	mkdir -p $(PREFIX)/include
	./inst.sh include/cm.h		$(PREFIX)	  0711	$(USER)	$(GROUP)
	./inst.sh include/common.h	$(PREFIX)	  0711	$(USER)	$(GROUP)
	./inst.sh include/form.h	$(PREFIX)	  0711	$(USER)	$(GROUP)
	./inst.sh include/menu.h	$(PREFIX)	  0711	$(USER)	$(GROUP)
	./inst.sh include/pick.h	$(PREFIX)	  0711	$(USER)	$(GROUP)
	./inst.sh include/view.h	$(PREFIX)	  0711	$(USER)	$(GROUP)
	./inst.sh menu			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh form			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh pick			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh ckeys			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh view			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh optsp			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh whence		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh rsh			$(BIN_DIR)	  4711	root	root
	./inst.sh enterchr		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh enterstr		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh iloan			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh stripansi		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh lf		$(BIN_DIR)		  0711	$(USER)	$(GROUP)
	mkdir -p $(BIN_DIR)
	cp -Rdup bin $(PREFIX)/bin
	mkdir -p $(LIB_DIR)
	cp $(LIB_NAME) $(LIB_DIR)/$(REAL_NAME)
	echo `test/iso8601.sh` $(LIB_DIR)/$(REAL_NAME) >>manifest.txt
	ln -sf $(LIB_DIR)/$(REAL_NAME) $(LIB_DIR)/$(LIB_NAME)
	echo `test/iso8601.sh` $(LIB_DIR)/$(LIB_NAME) >>manifest.txt
	ln -sf $(LIB_DIR)/$(REAL_NAME) $(LIB_DIR)/$(SONAME)
	echo `test/iso8601.sh` $(LIB_DIR)/$(SONAME) >>manifest.txt
	cp CMenu.conf /etc/ld.so.conf.d
	echo `test/iso8601.sh` /etc/ld.so.conf.d/CMenu.conf >>manifest.txt
	ldconfig
	./inst.sh -p
	./inst.sh -l
	./inst.sh -m

uninstall:
	rm -fv	$(BIN_DIR)/menu	\
			$(BIN_DIR)/form	\
			$(BIN_DIR)/pick	\
			$(BIN_DIR)/ckeys	\
			$(BIN_DIR)/view \
			$(BIN_DIR)/rsh \
			$(BIN_DIR)/ush \
			$(BIN_DIR)/enterchr \
			$(BIN_DIR)/enterstr \
			$(BIN_DIR)/stripansi \
			$(BIN_DIR)/lf \
			$(LIB_DIR)/libcm.so
