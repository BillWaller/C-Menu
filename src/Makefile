# @file Makefile
# @brief Makefile for C-Menu application suite
# @author Bill Waller
# Copyright (c) 2026
# MIT License
# billxwaller@gmail.com
# @date 2026-06-10
#
ifneq ($(CMENU_HOME),)
	PREFIX:=$(HOME)/menuapp
endif
DISPLAY_MANIFEST=0
BUILD_TYPE=Debug
UI="-DUAL_UI"
UI="-DNCURSES_UI"
ifeq ($(BUILD_TYPE), Release)
	CFLAGS=-Ofast -s -fPIC -pthread $(UI)
	EXTRA_CFLAGS=-flto
	LINKOPT= -pthread -s
else
#	-DDEBUG_IMMEDOK
	DEBUG_FLAGS=-DDEBUG_RESIZE -DDEBUG_LOG -DDEBUG_LAYOUT
	CFLAGS:=-O0 -g3 -fPIC $(DEBUG_FLAGS) -pthread -fno-omit-frame-pointer \
			-rdynamic $(UI)
	LINKOPT= -g3 -fno-omit-frame-pointer -rdynamic -pthread
endif
INCLUDE_DIRS:=-I./include -I./ui
CFLAGS+=$(INCLUDE_DIRS)
PACKAGES= panelw ncursesw
CFLAGS+=$(shell pkg-config --cflags $(PACKAGES))
CFLAGS+=-Wall -Wextra -Wshadow -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Werror -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-value -Wno-unused-const-variable -Wno-unused-macros
MAJOR=0
MINOR=2
PATCH=9
VERSION:=$(MAJOR).$(MINOR).$(PATCH)
SYSDATE:=$(shell date -u +"%Y-%m-%dT%H:%M:%SZ")
LIB_DIR:=/usr/lib64
LIB_SHARED:=libcm.so
LIB_STATIC:=libcm.a
INSTALL_LIB_DIR:=$(PREFIX)/lib64
BUILD_DIR:=$(CURDIR)
INSTALL_BIN_DIR:=$(PREFIX)/bin
INSTALL_MAN_DIR:=$(PREFIX)/man/man1
RPATH:=./:$(INSTALL_LIB_DIR):$(LIB_DIR)
PAM_DIR:=/etc/pam.d
CX:=-std=c23
CC:=gcc $(CX) $(INCLUDE_DIRS) $(CFLAGS)

BINEXES:=menu form pick view ckeys \
		whence rsh rsh_static rsh_pam \
		iloan amort enterchr enterstr lf stripansi detach

MANPAGES:=cmenu.1.gz lf.1.gz rsh.1.gz

OTHER:=CMenu.conf

LIB_SRCS=dwin.c futil.c scriou.c exec.c sig.c
ifeq ($(UI), "-DUAL_UI")
	LIB_SRCS+=ui/ui_ncurses.c ui/ui_ncurses_draw.c ui/ui_ncurses_input.c
endif
LIB_OBJS = $(LIB_SRCS:.c=.o)

COMMON_HDRS:=include/cm.h include/common.h include/menu.h \
		include/form.h include/pick.h include/view.h include/version.h \
		include/version.h.in
COMMON_SRCS:=menu_engine.c parse_menu_desc.c curskeys.c \
			fields.c form_engine.c init.c init_view.c mem.c \
			pick_engine.c popups.c view_engine.c
ifeq ($(UI), "-DUAL_UI")
	COMMON_HDRS+=include/ui_backend.h ui/ui_ncurses_internal.h
	COMMON_SRCS+=ui/ui_ncurses.c ui/ui_ncurses_draw.c ui/ui_ncurses_input.c
	NCFLAGS:=$(CFLAGS) -DNCURSES_UI
endif

COMMON_OBJS=$(COMMON_SRCS:.c=.o)

LDFLAGS:=$(LINKOPT) -L. -lcm -lm $(shell pkg-config --libs $(PACKAGES)) \
		 -Wl,-rpath,$(RPATH)
SHARED_LDFLAGS:=-Wl,-ffunction-sections \
		-Wl,-fdata-sections \
		-Wl,--gc-sections
# -Wl,-Map=output.map

.PHONY: all config help install man install_man clean uninstall helgrind valgrind

all:	$(LIB_SHARED) $(LIB_STATIC) $(BINEXES) CMenu.conf
	@echo Make Complete

config:
	@echo "     Build Date $(SYSDATE)"
	@echo "     Build Type $(BUILD_TYPE)"
	@echo "             UI $(UI)"
	@echo "         PREFIX $(PREFIX)"
	@echo "      BUILD_DIR $(BUILD_DIR)"
	@echo "        VERSION $(VERSION)"
	@echo "             CC $(CC)"
	@echo "             CX $(CX)"
	@echo "         CFLAGS $(CFLAGS)"
	@echo "        NCFLAGS $(NCFLAGS)"
	@echo "        LDFLAGS $(LDFLAGS)"
	@echo "        LINKOPT $(LINKOPT)"
	@echo "          RPATH $(RPATH)"
	@echo "     LIB_SHARED $(LIB_SHARED)"
	@echo "     LIB_STATIC $(LIB_STATIC)"
	@echo "INSTALL_LIB_DIR $(INSTALL_LIB_DIR)"
	@echo "    INCLUDE_DIR $(INCLUDE_DIR)"
	@echo "INSTALL_BIN_DIR $(INSTALL_BIN_DIR)"
	@echo "INSTALL_MAN_DIR $(INSTALL_MAN_DIR)"
	@echo "        LIB_DIR $(LIB_DIR)"
	@echo "        PAM_DIR $(PAM_DIR)"
	@echo "    COMMON_HDRS $(COMMON_HDRS)"
	@echo "    COMMON_SRCS $(COMMON_SRCS)"
	@echo "    COMMON_OBJS $(COMMON_OBJS)"
	@echo "        UI_OBJS $(UI_OBJS)"

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

menu:	menu.c $(COMMON_OBJS) $(COMMON_HDRS) $(LIB_SHARED)
	$(CC) $(NCFLAGS) $(COMMON_OBJS) menu.c -o $@ $(LDFLAGS)

form:	menu
	ln -sf menu form
pick:	menu
	ln -sf menu pick
view:	menu
	ln -sf menu view
ckeys:	menu
	ln -sf menu ckeys

man: $(MANPAGES)

rsh:	rsh.c
	$(CC) $< -o $@

rsh_static:	rsh.c
	$(CC) -static $< -o $@

rsh_pam:	rsh.c
	$(CC) -DRSH_PAM $< -o $@ -lpam -lpam_misc

lf: lf.c $(LIB_SHARED)
	$(CC) $< -o $@ $(LDFLAGS)

%:	%.c $(LIB_SHARED)
	$(CC) $< -o $@ $(LDFLAGS)

%.o:	%.c
	$(CC) 	-c $< -fPIC -o $@

iloan:	iloan.c
	$(CC) 	$< -lm -o $@

amort:	amort.c
	$(CC) 	$< -lm -o $@

whence:	whence.c
	$(CC) 	$< -o $@

# enterchr:	enterchr.c
#	$(CC) 	$< -o $@

#enterstr:	enterstr.c
#	$(CC) 	$< -o $@

$(LIB_SHARED): $(LIB_OBJS)
	$(CC) 	-shared -o $@ $^ $(SHARED_LDFLAGS)

$(LIB_STATIC): $(LIB_OBJS)
	ar rcs $@ $^

CMenu.conf:
	echo $(INSTALL_LIB_DIR) >CMenu.conf

cmenu.1.gz:	../docs/cmenu.md
	pandoc -f markdown-smart -standalone --to man ../docs/cmenu.md -o cmenu.1
	gzip -f cmenu.1

lf.1.gz:	../docs/lf.md
	pandoc -f markdown-smart -standalone --to man ../docs/lf.md -o lf.1
	gzip -f lf.1

rsh.1.gz:	../docs/rsh.md
	pandoc -f markdown-smart -standalone --to man ../docs/rsh.md -o rsh.1
	gzip -f rsh.1

clean:
	rm -rf $(BINEXES) $(CFLOW_OUTPUT) .user_id .group_id core scan-build* \
	CMenu.conf manifest.txt manifest.ls perf.* x xx* *.[aeos] *.diff *.so \
	*.cflow *.gc.. *.out *.1 *.1.gz CMakeFiles cmake_install.cmake \
   	CMakeCache.txt CMakefile CMAKE_BUILD_TYPE=Debug gdb.log ui/*.o

helgrind: lf
	valgrind --tool=helgrind --log-file=helgrind.out -s ./lf -H -L -T6 /home/bill 1>/dev/null 2>/dev/null

valgrind: menu
	valgrind -v \
		--tool=memcheck \
    	--log-file=valgrind.out \
		--leak-check=full \
		--show-leak-kinds=definite \
		--show-reachable=no \
		--num-callers=30 \
		--leak-resolution=high \
		--track-origins=yes \
		--keep-debuginfo=yes \
		--suppressions=valgrind.supp \
		--gen-suppressions=all \
		./menu

install:
	rm -f	$(INSTALL_LIB_DIR)/libcm*
	install -CDp 	$(COMMON_HDRS)	-t $(PREFIX)/include
	install -CDp	menu whence lf detach enterchr enterstr \
		iloan amort stripansi -t $(INSTALL_BIN_DIR)
	install -Dp -o root -g root -m 6711	rsh rsh_static rsh_pam -t $(INSTALL_BIN_DIR)
	install -CDp $(LIB_STATIC) -t $(INSTALL_LIB_DIR)
	install	-CDp $(LIB_SHARED) -T $(INSTALL_LIB_DIR)/$(LIB_SHARED).$(VERSION)
	install -CDp CMenu.conf -t /etc/ld.so.conf.d
	install -CDp rsh-auth-pc -t $(PAM_DIR) -o root -g root -m 644
	ln	-sf	$(PAM_DIR)/rsh-auth-pc $(PAM_DIR)/rsh-auth
	ln	-sf	$(INSTALL_LIB_DIR)/$(LIB_SHARED).$(VERSION) $(INSTALL_LIB_DIR)/$(LIB_SHARED)
	ln	-sf	$(INSTALL_LIB_DIR)/$(LIB_SHARED).$(VERSION) $(INSTALL_LIB_DIR)/$(LIB_SHARED).$(MAJOR)
	ln	-sf	$(INSTALL_BIN_DIR)/menu $(INSTALL_BIN_DIR)/form
	ln	-sf	$(INSTALL_BIN_DIR)/menu $(INSTALL_BIN_DIR)/pick
	ln	-sf	$(INSTALL_BIN_DIR)/menu $(INSTALL_BIN_DIR)/view
	ln	-sf	$(INSTALL_BIN_DIR)/menu $(INSTALL_BIN_DIR)/ckeys
	ldconfig
	./manifest.sh
ifeq ($(DISPLAY_MANIFEST), 1)
	cat manifest.txt
endif

manifest:
	cat manifest.txt

install_man:
	install -CDp cmenu.1.gz	-t $(INSTALL_MAN_DIR)
	install -CDp lf.1.gz	-t $(INSTALL_MAN_DIR)
	install -CDp rsh.1.gz	-t $(INSTALL_MAN_DIR)

uninstall:
			$(INSTALL_BIN_DIR)/menu \
			$(INSTALL_BIN_DIR)/form \
			$(INSTALL_BIN_DIR)/pick \
			$(INSTALL_BIN_DIR)/view \
			$(INSTALL_BIN_DIR)/ckeys \
			$(INSTALL_BIN_DIR)/detach \
			$(INSTALL_BIN_DIR)/rsh \
			$(INSTALL_BIN_DIR)/rsh_static \
			$(INSTALL_BIN_DIR)/rsh_pam \
			$(INSTALL_BIN_DIR)/ush \
			$(INSTALL_BIN_DIR)/enterchr \
			$(INSTALL_BIN_DIR)/enterstr \
			$(INSTALL_BIN_DIR)/stripansi \
			$(INSTALL_BIN_DIR)/lf \
			$(INSTALL_LIB_DIR)/libcm.so
