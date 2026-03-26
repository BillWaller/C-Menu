# @file Makefile
# @brief Makefile for C-Menu application suite
# @author Bill Waller
# Copyright (c) 2025
# MIT License
# billxwaller@gmail.com
# @date 2026-03-25
#
# === BEGIN USER SECTION       ===
#
# Before using this makefile, please EDIT THE FOLLOWING VARIABLES to match your 
# system configuration. The lines below assume that you have a user named "bill" 
# and a group named "bill". If your user name is not "bill", or your group name 
# is not "bill", please update the USER and GROUP variables accordingly. 
#
# Edit the four lines below to match your system configuration:
#
USER=bill
GROUP=bill
HOME=/home/$(USER)
PREFIX=$(HOME)/menuapp

# Uncomment the following line to enable logging for RSH
# RSH_CFLAGS = -DRSH_LOG
# 
# -DRSH_LOG: If you enable system logging for RSH, you will need to ensure that
# your system is configured to allow the RSH binary to write to the system log. 
# This may involve setting appropriate permissions for the RSH binary and 
# ensuring that it is executed with sufficient privileges to write to the log. 
# Additionally, you may need to configure your system's syslog daemon to accept 
# log messages from the RSH binary and to route those messages to the 
# appropriate log file or log facility. Please consult your system's 
# documentation for more information on how to configure logging for RSH.
#
# Uncomment the following line to enables SSH authentication for RSH
# RSH_CFLAGS = $(RSH_CFLAGS) -DRSH_SSH
#
# -DRSH_SSH: If you enable SSH authentication for RSH, you will need to ensure
# that libssh is installed on your system and that the header files and 
# libraries are accessible to the compiler. You may need to install the libssh 
# development package (e.g., libssh-dev on Debian-based systems) to get the 
# necessary files for compilation. Additionally, you may need to adjust the 
# include and library paths in the makefile to point to the correct locations 
# for the libssh headers and libraries on your system.
#
# Uncomment the following line to enable static linking of rsh
# RSH_LD = -static
# 
# RSH_LD and -DRSH_SSH: If compiling with RSH_LD -static and -DRSH_SSH, you will
# need to ensure that libssh.a is available on your system and that it is 
# compatible with static linking. This may require additional dependencies and 
# configuration, so please consult the documentation for libssh and your 
# system's package manager for more information.
#
CFLAGS=-O2
#
# Uncomment the following line to enable Debug build flags
CFLAGS=-O0 -ggdb -rdynamic -DDEBUG
#
# === END USER SECTION         ===
#
# There should be no need to edit below this line unless you are developing 
# C-Menu.
#
MAJOR=0
MINOR=2
PATCH=9
VERSION=$(MAJOR).$(MINOR).$(PATCH)

SYSDATE := $(date -u +"%Y-%m-%dT%H:%M:%SZ")

CC=gcc -Wall -Wextra -I./include $(CFLAGS)
CX=-std=gnu23
CX=-std=c23

LIB_SHARED=libcm.so
SONAME=$(LIB_SHARED).$(MAJOR)
REAL_NAME=$(LIB_SHARED).$(VERSION)
LIB_STATIC=libcm.a
LIB_NAME=$(LIB_SHARED)
LIB_DIR=$(PREFIX)/lib64
BIN_DIR=$(PREFIX)/bin
INC_DIR=$(PREFIX)
LIB_PATH=.:$(LIB_DIR)

BINEXES=menu form pick view ckeys whence rsh iloan enterchr \
		enterstr lf optsp stripansi

LIB_SRCS=dwin.c futil.c scriou.c exec.c sig.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

COMMON_HDRS=include/cm.h include/common.h include/menu.h \
		include/form.h include/pick.h include/view.h include/version.h
COMMON_SRCS=menu_engine.c parse_menu_desc.c curskeys.c \
			fields.c form_engine.c init.c init_view.c mem.c \
			opts.c pick_engine.c popups.c view_engine.c
COMMON_OBJS=$(COMMON_SRCS:.c=.o)

LIBS = -L. -lcm -lncursesw -ltinfo -lm -Wl,-rpath,$(LIB_PATH)

all:	$(LIB_SHARED) $(LIB_STATIC) $(BINEXES) CMenu.conf
	@echo Make Complete

menu:	menu.o $(COMMON_OBJS) $(COMMON_HDRS) $(LIB_NAME)
	$(CC) $(COMMON_OBJS) menu.o -o $@ $(LIBS)

form:	menu
	ln -sf menu form
pick:	menu
	ln -sf menu pick
view:	menu
	ln -sf menu view
ckeys:	menu
	ln -sf menu ckeys

enterchr:	include/cm.h enterchr.o $(LIB_NAME)
	$(CC) enterchr.o -o $@ $(LIBS)

optsp:	include/cm.h optsp.o opts.o $(LIB_NAME)
	$(CC) optsp.o opts.o -o $@ $(LIBS)

enterstr:	include/cm.h enterstr.o $(LIB_NAME)
	$(CC) enterstr.o -o $@ $(LIBS)

whence:	include/cm.h whence.o $(LIB_NAME)
	$(CC) whence.o -o $@ $(LIBS)

iloan:	iloan.o $(LIB_NAME)
	$(CC) iloan.o -o $@ $(LIBS)

rsh:	include/cm.h rsh.o
	$(CC) $(RSH_CFLAGS) rsh.o -o $@ -lssh

lf: include/cm.h	lf.o $(LIB_NAME)
	$(CC) lf.o -o $@ $(LIBS)

stripansi:	include/cm.h stripansi.o $(LIB_NAME)
	$(CC) stripansi.o -o $@ $(LIBS)

CMenu.conf:
	echo $(LIB_DIR) >CMenu.conf

$(LIB_SHARED): $(LIB_OBJS)
	$(CC) 	-shared -o $@ $^

$(LIB_STATIC): $(LIB_OBJS)
	ar rcs $@ $^

dwin.o:	dwin.c
	$(CC) 	-c $< -fPIC -o $@

futil.o:	futil.c
	$(CC) 	-c $< -fPIC -o $@

popups.o:	popups.c
	$(CC) 	-c $< -fPIC -o $@

scriou.o:	scriou.c
	$(CC) 	-c $< -fPIC -o $@

exec.o:	exec.c
	$(CC) 	-c $< -fPIC -o $@

sig.o:	sig.c
	$(CC) 	-c $< -fPIC -o $@

%.o:	%.c
	$(CC) 	-c $< -o $@

clean:
	rm -rf core *.o $(BINEXES) make.out $(CFLOW_OUTPUT) *.o libcm.a \
		libcm.so *.cflow scan-build scan-build.out $(LIB_SHARED) \
		CMenu.conf x xx xxx *.out .user_id .group_id manifest.txt \
		manifest.ls

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
	./inst.sh optsp			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh whence		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh rsh			$(BIN_DIR)	  4711	root	root
	./inst.sh enterchr		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh enterstr		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh iloan			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh stripansi		$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	./inst.sh lf			$(BIN_DIR)	  0711	$(USER)	$(GROUP)
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	rm -f	 $(LIB_DIR)/libcm*
	cp	 $(LIB_STATIC) $(LIB_DIR)
	echo	$(SYSDATE) $(LIB_DIR)/$(LIB_STATIC) >>manifest.txt
	cp	 $(LIB_SHARED) $(LIB_DIR)/$(REAL_NAME)
	echo	$(SYSDATE) $(LIB_DIR)/$(REAL_NAME) >>manifest.txt
	ln -sf	 $(LIB_DIR)/$(REAL_NAME) $(LIB_DIR)/$(LIB_SHARED)
	echo	$(SYSDATE) $(LIB_DIR)/$(LIB_SHARED) >>manifest.txt
	ln -sf	 $(LIB_DIR)/$(REAL_NAME) $(LIB_DIR)/$(SONAME)
	echo	$(SYSDATE) $(LIB_DIR)/$(SONAME) >>manifest.txt
	ln -sf	 $(BIN_DIR)/menu $(BIN_DIR)/form
	ln -sf	 $(BIN_DIR)/menu $(BIN_DIR)/pick
	ln -sf	 $(BIN_DIR)/menu $(BIN_DIR)/view
	ln -sf	 $(BIN_DIR)/menu $(BIN_DIR)/ckeys
	cp CMenu.conf /etc/ld.so.conf.d
	echo `test/iso8601.sh` /etc/ld.so.conf.d/CMenu.conf >>manifest.txt
	ldconfig
	./inst.sh -p
	./inst.sh -l
	./inst.sh -m

uninstall:
			$(BIN_DIR)/menu \
			$(BIN_DIR)/form \
			$(BIN_DIR)/pick \
			$(BIN_DIR)/view \
			$(BIN_DIR)/ckeys \
			$(BIN_DIR)/rsh \
			$(BIN_DIR)/ush \
			$(BIN_DIR)/enterchr \
			$(BIN_DIR)/enterstr \
			$(BIN_DIR)/stripansi \
			$(BIN_DIR)/lf \
			$(LIB_DIR)/libcm.so
