# @file Makefile
# @brief Makefile for C-Menu application suite
# @author Bill Waller
# Copyright (c) 2025
# MIT License
# billxwaller@gmail.com
# @date 2026-03-25
#
# Before using this makefile, please EDIT THE FOLLOWING VARIABLES to match your 
# system configuration. The lines below assume that you have a user named "bill" 
# and a group named "bill". If your user name is not "bill", or your group name 
# is not "bill", please update the USER and GROUP variables accordingly. 
#
# Please check whether the -g3 or -ggdb debugging lines are active. If you are 
# not using a debugger, you may want to comment out the -g3 or -ggdb flags to 
# reduce the size of the compiled binaries and improve performance.
#
# Consider whether you want to use Makefile or CMake for your build system.
# Makefile is a simple and widely used build system that is suitable for small 
# to medium-sized projects. CMake is a more powerful and flexible build system 
# that can handle larger and more complex projects, and it can generate build 
# files for multiple platforms and build systems. If you are working on a small 
# project or prefer a simpler build system, Makefile may be the better choice. 
# If you are working on a larger project or need more advanced features, CMake 
# may be the better choice. Ultimately, the decision between Makefile and CMake 
# depends on your specific needs and preferences. You have both systems here,
# so you can choose which one to use based on your project requirements advanced
# features, and personal preferences. If you are new to build systems or prefer 
# appropriate simplicity, Makefile may be the better choice. If you need more 
# advanced features, cross-platform support, or better integration with IDEs,
# CMake may be the better choice. Consider your project requirements, team 
# preferences, and the learning curve associated with each build system when 
# making your decision.
#
# Edit the four lines below to match your system configuration:
#
# It's better not to use "USER=$(id -un)" and "GROUP=$(id -gn)" in the makefile, 
# as it can lead to issues with permissions and ownership when installing the 
# binaries and libraries. Instead, it's recommended to set the USER and GROUP 
# variables to specific values that match your system configuration. This way, 
# you can ensure that the installed files have the correct ownership and 
# permissions, and you can avoid potential issues with file access and security.
#
# Please update the USER and GROUP variables to match your system configuration 
# before running the make command.
USER=bill
GROUP=bill
PREFIX=$(CMENU_HOME)
DISPLAY_MANIFEST ?= 0
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
# Uncomment the following line to enable SSH authentication for RSH
# RSH_CFLAGS = -DRSH_SSH
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
CFLAGS = -Ofast -flto -pthread
LINKOPT = -pthread -flto
#   -DLF_DEBUG \
# CFLAGS = -Ofast \
#    -march=native -flto -pthread -fno-omit-frame-pointer
#
# Uncomment the following line to enable NCurses Debugging with immedok(true).
# CFLAGS=-O0 -ggdb -rdynamic -DDEBUG_LOG -DDEBUG_IMMEDOK -DDEBUG_RESIZE
# -DEBUG_IMMEDOK will cause the screen to update immediately after each change, 
#  which can be helpful for debugging screen drawing issues. However, it may 
#  also cause performance issues, so it is not recommended for production 
#  builds.
#
# -rdynamic passes -export-dynamic to the linker, which allows the symbols from
#   the executable to be available for dynamic linking. This is necessary for 
#   debugging with tools like gdb, as it allows gdb to access the symbols and 
#   functions defined in the executable. Without -rdynamic, gdb may not be able 
#   to access the symbols, making it more difficult to debug the program. 
#   However, it may also increase the size of the executable and may not be 
#   necessary for production builds, so it is typically used only for debugging 
#   purposes.
#
# Standard Debugging Flags
CFLAGS=-O0 -g3 -pthread -fno-omit-frame-pointer -rdynamic -DDEUBG_IMMEDOK
# LINKOPT = -pthread
# CFLAGS=-DDEBUG_RESIZE -DDEUBG_IMMEDOK -DDEBUG_LOG
#
# Helgrind Debugging Flags
# CFLAGS=-O0 -g -pthread
#
# CFLAGS=-g -O3 -fsanitize=thread
#
# CFLAGS=-g -O3 -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer \
# -fno-sanitize-recover=all
#
#  Uncomment the following line to enable Address Sanitizer, Leak Sanitizer, and 
#  Undefined Behavior Sanitizer.
# -fsanitize=address fsanitize=leak
# -fsanitize=undefined
# -fno-omit-frame-pointer
# -fno-sanitize-recover=all
#
# Uncomment the following line to enable debugging
# CFLAGS=-O0 -g3 -pthread
# -rdynamic
# -DDEBUG_IMMEDOK
# Additional debugging flags for C-Menu
#	-DDEBUG_IMMEDOK -DDEBUG_RESIZE -DDEBUG_LOG
# Add -gp for profiling
# CFLAGS=-O0 -gp -rdynamic -DDEBUG_IMMEDOK -DDEBUG_RESIZE -DDEBUG_LOG
# Additional flags for debugging with Address Sanitizer
# ASAN_FLAGS=-no-pie -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer 
# -fno-sanitize-recover=all
# CFLAGS=-O3 -g -fno-omit-frame-pointer -rdynamic
# $(ASAN_FLAGS)
# LIBASAN=-lasan $(ASAN_FLAGS)
#	$(ASAN_FLAGS)
#	-fsanitize=address -fsanitize=leak -fno-omit-frame-pointer \
#	-fno-sanitize-recover=all
# LIBASAN=-lasan $(ASAN_FLAGS)
# export LD_PRELOAD=/usr/lib64/libasan.so.8
# export ASAN_OPTIONS="log_path=asan.log:detect_leaks=1:color=never"
#
# Uncomment the following line to enable coverage.
# CFLAGS=--coverage -O0 -ggdb -rdynamic -DNCDEBUG
#
# There should be no need to edit below this line unless you are developing 
# C-Menu.
#
MAJOR=0
MINOR=2
PATCH=9
VERSION=$(MAJOR).$(MINOR).$(PATCH)

SYSDATE := $(shell date -u +"%Y-%m-%dT%H:%M:%SZ")

CC=gcc -Wall -Wextra -I./include $(CFLAGS)
CX=-std=gnu23
CX=-std=c23

LIB_DIR=/usr/lib64

LIB_SHARED=libcm.so
SONAME=$(LIB_SHARED).$(MAJOR)
REAL_NAME=$(LIB_SHARED).$(VERSION)
LIB_STATIC=libcm.a
LIB_NAME=$(LIB_SHARED)
INSTALL_LIB_DIR=$(PREFIX)/lib64
BUILD_DIR=$(CMENU_SRC)
BIN_DIR=$(PREFIX)/bin
MAN_DIR=$(PREFIX)/man/man1
INC_DIR=$(PREFIX)
LIB_PATH=.:$(INSTALL_LIB_DIR):$(LIB_DIR)
PAM_DIR=/etc/pam.d

BINEXES=menu form pick view ckeys whence rsh iloan amort enterchr \
		enterstr lf stripansi

MANPAGES=cmenu.1.gz lf.1.gz rsh.1.gz

LIB_SRCS=dwin.c futil.c scriou.c exec.c sig.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

COMMON_HDRS=include/cm.h include/common.h include/menu.h \
		include/form.h include/pick.h include/view.h include/version.h
COMMON_SRCS=menu_engine.c parse_menu_desc.c curskeys.c \
			fields.c form_engine.c init.c init_view.c mem.c \
			pick_engine.c popups.c view_engine.c
COMMON_OBJS=$(COMMON_SRCS:.c=.o)

LIBS = $(LIBASAN) $(LINKOPT) -L. -lcm -lncursesw -ltinfo -lm -luring -Wl,-rpath,$(LIB_PATH)

.PHONY: all install install_man clean uninstall helgrind valgrind

all:	$(LIB_SHARED) $(LIB_STATIC) $(BINEXES) CMenu.conf
	@echo Build Date: $(SYSDATE)
	@echo CMENU_HOME = $(CMENU_HOME)
	@echo Make Complete

config:
	@echo Build Date: $(SYSDATE)
	@echo VERSION = $(VERSION)
	@echo CMENU_HOME = $(CMENU_HOME)
	@echo USER = $(USER)
	@echo GROUP $(GROUP)
	@echo PREFIX = $(CMENU_HOME)
	@echo CFLAGS = $(CFLAGS)
	@echo LINKOPT = $(LINKOPT)
	@echo CC = $(CC)
	@echo CX = $(CX)-std=gnu23
	@echo LIB_DIR = $(LIB_DIR)
	@echo LIB_SHARED = $(LIB_SHARED)
	@echo SONAME = $(SONAME)
	@echo REAL_NAME = $(REAL_NAME)
	@echo LIB_STATIC = $(LIB_STATIC)
	@echo LIB_NAME = $(LIB_NAME)
	@echo INSTALL_LIB_DIR = $(INSTALL_LIB_DIR)
	@echo BUILD_DIR = $(BUILD_DIR)
	@echo BIN_DIR = $(BIN_DIR)
	@echo MAN_DIR = $(MAN_DIR)
	@echo INC_DIR = $(INC_DIR)
	@echo LIB_PATH = $(LIB_PATH)
	@echo PAM_DIR = $(PAM_DIR)
	@echo BINEXES = $(BINEXES)
	@echo MANPAGES $(MANPAGES)
	@echo LIBS = $(LIBS)

help:
	@echo "Available targets:"
	@echo "  config        - Display Configuration"
	@echo "  all           - Build the library and all binaries"
	@echo "  install       - Install binaries, libraries, and headers"
	@echo "  man           - Build Manual Pages"
	@echo "  install_man   - Install Manual Pages"
	@echo "  clean         - Remove build artifacts"
	@echo "  uninstall     - Remove installed binaries and libraries"
	@echo "  manifest      - Display manifest of installed files"
	@echo "  valgrind      - run valgrind on selected executable"
	@echo "  helgrind      - run helgrind on selected executable"

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

man: $(MANPAGES)

enterchr:	enterchr.o $(LIB_NAME)
	$(CC) $< -o $@ $(LIBS)

enterstr:	enterstr.o $(LIB_NAME)
	$(CC) $< -o $@ $(LIBS)

whence:	whence.o $(LIB_NAME)
	$(CC) $< -o $@ $(LIBS)

iloan:	iloan.o $(LIB_NAME)
	$(CC) $< -o $@ $(LIBS)

# amort:	amort.o $(LIB_NAME)
# 	$(CC) $< -o $@ $(LIBS)

rsh:	rsh.o
	$(CC) $(RSH_CFLAGS) $< -o $@ -lpam -lpam_misc

lf: lf.c $(LIB_NAME)
	$(CC) $< -o $@ $(LIBS)

%:	%.o $(LIB_NAME)
	$(CC) $< -o $@ $(LIBS)

cmenu.1.gz:	../docs/cmenu.md
	pandoc -f markdown-smart -standalone --to man ../docs/cmenu.md -o cmenu.1
	gzip -f cmenu.1

lf.1.gz:	../docs/lf.md
	pandoc -f markdown-smart -standalone --to man ../docs/lf.md -o lf.1
	gzip -f lf.1

rsh.1.gz:	../docs/rsh.md
	pandoc -f markdown-smart -standalone --to man ../docs/rsh.md -o rsh.1
	gzip -f rsh.1

stripansi:	include/cm.h stripansi.o $(LIB_NAME)
	$(CC) stripansi.o -o $@ $(LIBS)

CMenu.conf:
	echo $(INSTALL_LIB_DIR) >CMenu.conf

$(LIB_SHARED): $(LIB_OBJS)
	$(CC) 	-shared -o $@ $^

$(LIB_STATIC): $(LIB_OBJS)
	ar rcs $@ $^

%.o:	%.c
	$(CC) 	-c $< -fPIC -o $@

clean:
	rm -rf core *.o $(BINEXES) $(CFLOW_OUTPUT) *.[aeos] *.diff \
		*.so *.cflow scan-build* CMenu.conf x xx* *.out \
	   	.user_id .group_id manifest.txt manifest.ls *.gc.. \
		perf.* *.1 *.1.gz

test: snippets/strings_test1.c
	$(CC) -o test_strings snippets/strings_test1.c $(LIBS)

helgrind: lf
	valgrind --tool=helgrind --log-file=helgrind.out -s ./lf -H -L -T6 /home/bill 1>/dev/null 2>/dev/null

# Run Valgrind with the project suppression file.
# Any remaining output after applying suppressions indicates a real leak.
# See docs/valgrind.md for full details.
valgrind: lf
	valgrind -v \
    	--log-file=valgrind.out \
		--leak-check=full \
		--show-leak-kinds=all \
		--leak-resolution=high \
		--track-origins=yes \
		--suppressions=valgrind.supp \
		./lf -H -L -T6 /home/bill
# --tool=memcheck

install:
	rm -f	$(INSTALL_LIB_DIR)/libcm*
	install -CDp include/cm.h -t $(PREFIX)/include
	install -CDp include/common.h -t $(PREFIX)/include
	install -CDp include/form.h -t $(PREFIX)/include
	install -CDp include/menu.h -t $(PREFIX)/include
	install -CDp include/pick.h -t $(PREFIX)/include
	install -CDp include/view.h -t $(PREFIX)/include
	install -CDp menu	-t $(BIN_DIR)
	install -CDp whence	-t $(BIN_DIR)
	install -Dp  rsh	-t $(BIN_DIR)	-o root -g root -m 6711
	install -CDp enterchr	-t $(BIN_DIR)
	install -CDp enterstr	-t	$(BIN_DIR)
	install -CDp iloan	-t $(BIN_DIR)
	install -CDp amort	-t $(BIN_DIR)
	install -CDp stripansi	-t $(BIN_DIR)
	install -CDp rsh-auth-pc -t $(PAM_DIR) -o root	-g root -m 0644
	install -CDp $(LIB_STATIC) -t $(INSTALL_LIB_DIR)
	install	-CDp $(LIB_SHARED) -T $(INSTALL_LIB_DIR)/$(REAL_NAME)
	ln	-sf	$(INSTALL_LIB_DIR)/$(REAL_NAME) $(INSTALL_LIB_DIR)/$(LIB_SHARED)
	ln	-sf	$(INSTALL_LIB_DIR)/$(REAL_NAME) $(INSTALL_LIB_DIR)/$(SONAME)
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/form
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/pick
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/view
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/ckeys
	ln	-sf	$(PAM_DIR)/rsh-auth-pc $(PAM_DIR)/rsh-auth
	install -CDp CMenu.conf -t /etc/ld.so.conf.d
	ldconfig
	./manifest.sh
ifeq ($(DISPLAY_MANIFEST), 1)
	cat manifest.txt
endif

manifest:
	cat manifest.txt

install_man:
	install -CDp cmenu.1.gz	-t $(MAN_DIR)
	install -CDp lf.1.gz	-t $(MAN_DIR)
	install -CDp rsh.1.gz	-t $(MAN_DIR)

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
			$(INSTALL_LIB_DIR)/libcm.so
