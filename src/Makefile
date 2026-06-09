# @file Makefile
# @brief Makefile for C-Menu application suite
# @author Bill Waller
# Copyright (c) 2025
# MIT License
# billxwaller@gmail.com
# @date 2026-03-25
#
# Before using this makefile, please EDIT THE FOLLOWING VARIABLES to match your 
# system configuration.
#
#
ifeq ($(CMENU_HOME),)
	CMENU_HOME = $(HOME)/menuapp
endif
DISPLAY_MANIFEST ?= 0
# Uncomment the following line to enable logging for RSH
# RSH_CFLAGS = -DRSH_LOG
# 
# -DRSH_LOG: If you enable system logging for RSH, you will need to ensure that
# your system is configured to allow the RSH binary to write to the system log.
# If you use PAM authentication, you shouldn't need -DRSH_LOG, as PAM can be 
# onfigured to log authentication attempts directly.
#
# RSH_CFLAGS = -DRSH_SSH
#
# PAM authentication is generally recommended for RSH, as it provides better 
# security and is more widely supported. If you choose to use PAM 
# authentication, you can enable it by uncommenting the following line:
#
RSH_CFLAGS = -DRSH_PAM
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
CFLAGS=-O0 -g3 -pthread -fno-omit-frame-pointer -rdynamic
# -DDEUBG_IMMEDOK
# -DDEUBG_IMMEDOK
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

# CX=-std=gnu23
CX=-std=c23
CC=gcc $(CX) -Wall -Wextra -I./include $(CFLAGS)

LIB_DIR=/usr/lib64
LIB_SHARED=libcm.so
LIB_STATIC=libcm.a
CMENU_LIB_DIR=$(CMENU_HOME)/lib64
BUILD_DIR=$(CMENU_SRC)
BIN_DIR=$(CMENU_HOME)/bin
MAN_DIR=$(CMENU_HOME)/man/man1
INC_DIR=$(CMENU_HOME)
RPATH=.:$(CMENU_LIB_DIR):$(LIB_DIR)
PAM_DIR=/etc/pam.d

BINEXES=menu form pick view ckeys whence rsh iloan amort enterchr \
		enterstr lf stripansi detach

MANPAGES=cmenu.1.gz lf.1.gz rsh.1.gz

LIB_SRCS=dwin.c futil.c scriou.c exec.c sig.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

COMMON_HDRS=include/cm.h include/common.h include/menu.h \
		include/form.h include/pick.h include/view.h include/version.h
COMMON_SRCS=menu_engine.c parse_menu_desc.c curskeys.c \
			fields.c form_engine.c init.c init_view.c mem.c \
			pick_engine.c popups.c view_engine.c
COMMON_OBJS=$(COMMON_SRCS:.c=.o)

LIBS = $(LIBASAN) $(LINKOPT) -L. -lcm -lncursesw -ltinfo -lm -luring -Wl,-rpath,$(RPATH)

.PHONY: all config help install install_man clean uninstall helgrind valgrind

all:	$(LIB_SHARED) $(LIB_STATIC) $(BINEXES) CMenu.conf
	@echo Make Complete

config:
	@echo "   Build Date $(SYSDATE)"
	@echo "   CMENU_HOME $(CMENU_HOME)"
	@echo "    BUILD_DIR $(BUILD_DIR)"
	@echo "      VERSION $(VERSION)"
	@echo "           CC $(CC)"
	@echo "           CX $(CX)"
	@echo "       CFLAGS $(CFLAGS)"
	@echo "         LIBS $(LIBS)"
	@echo "      LINKOPT $(LINKOPT)"
	@echo "        RPATH $(RPATH)"
	@echo "   LIB_SHARED $(LIB_SHARED)"
	@echo "   LIB_STATIC $(LIB_STATIC)"
	@echo "CMENU_LIB_DIR $(CMENU_LIB_DIR)"
	@echo "      INC_DIR $(INC_DIR)"
	@echo "      BIN_DIR $(BIN_DIR)"
	@echo "      MAN_DIR $(MAN_DIR)"
	@echo "      LIB_DIR $(LIB_DIR)"
	@echo "      PAM_DIR $(PAM_DIR)"

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

menu:	menu.o $(COMMON_OBJS) $(COMMON_HDRS) $(LIB_SHARED)
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

enterchr:	enterchr.o $(LIB_SHARED)
	$(CC) $< -o $@ $(LIBS)

enterstr:	enterstr.o $(LIB_SHARED)
	$(CC) $< -o $@ $(LIBS)

whence:	whence.o $(LIB_SHARED)
	$(CC) $< -o $@ $(LIBS)

iloan:	iloan.o $(LIB_SHARED)
	$(CC) $< -o $@ $(LIBS)

rsh:	rsh.o
	$(CC) $(RSH_CFLAGS) $< -o $@ -lpam -lpam_misc

lf: lf.c $(LIB_SHARED)
	$(CC) $< -o $@ $(LIBS)

%:	%.o $(LIB_SHARED)
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

stripansi:	include/cm.h stripansi.o $(LIB_SHARED)
	$(CC) stripansi.o -o $@ $(LIBS)

detach:	detach.o $(LIB_SHARED)
	$(CC) detach.o -o $@ $(LIBS)

CMenu.conf:
	echo $(CMENU_LIB_DIR) >CMenu.conf

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
	rm -f	$(CMENU_LIB_DIR)/libcm*
	install -CDp include/cm.h -t $(CMENU_HOME)/include
	install -CDp include/common.h -t $(CMENU_HOME)/include
	install -CDp include/form.h -t $(CMENU_HOME)/include
	install -CDp include/menu.h -t $(CMENU_HOME)/include
	install -CDp include/pick.h -t $(CMENU_HOME)/include
	install -CDp include/view.h -t $(CMENU_HOME)/include
	install -CDp menu	-t $(BIN_DIR)
	install -CDp whence	-t $(BIN_DIR)
	install -CDp lf     -t $(BIN_DIR)
	install -Dp  rsh	-t $(BIN_DIR)	-o root -g root -m 6711
	install -CDp enterchr	-t $(BIN_DIR)
	install -CDp enterstr	-t	$(BIN_DIR)
	install -CDp iloan	-t $(BIN_DIR)
	install -CDp amort	-t $(BIN_DIR)
	install -CDp stripansi	-t $(BIN_DIR)
	install -CDp detach		-t $(BIN_DIR)
	install -CDp rsh-auth-pc -t $(PAM_DIR) -o root	-g root -m 0644
	install -CDp $(LIB_STATIC) -t $(CMENU_LIB_DIR)
	install	-CDp $(LIB_SHARED) -T $(CMENU_LIB_DIR)/$(LIB_SHARED).$(VERSION)
	install -CDp CMenu.conf -t /etc/ld.so.conf.d
	ln	-sf	$(CMENU_LIB_DIR)/$(LIB_SHARED).$(VERSION) $(CMENU_LIB_DIR)/$(LIB_SHARED)
	ln	-sf	$(CMENU_LIB_DIR)/$(LIB_SHARED).$(VERSION) $(CMENU_LIB_DIR)/$(LIB_SHARED).$(MAJOR)
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/form
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/pick
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/view
	ln	-sf	$(BIN_DIR)/menu $(BIN_DIR)/ckeys
	ln	-sf	$(PAM_DIR)/rsh-auth-pc $(PAM_DIR)/rsh-auth
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
			$(CMENU_LIB_DIR)/libcm.so
