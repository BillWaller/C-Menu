#!/bin/bash

IFS="
"
files="ckeys.c
dgraph.c
display_curses_keys.c
dwin.c
enterchr.c
fform.c
fields.c
file.c
formexec.c
formwrite.c
fpaint.c
futil.c
iloan.c
init1-bak.c
init1.c
init2-bak.c
init2.c
init-bak.c
init.c
init_menu.c
init_pick.c
init_view.c
init-x-bak.c
log.c
menu-bak.h
menu.c
menu_engine.c
menu.h
mview.c
paintexec.c
paintwrite.c
parse_menu_desc.c
pick.c
pick_engine.c
rsh.c
scriou.c
usage.c
view.c
view_engine.c
whence.c"
for f in $files; do
    sed 's/init_opt/init/g' "$f" >tmp/"$f"
done

#
#    s/LineStructlen/line_len/g
#    s/dst_LineStructptr/dst_line_ptr/g
#    s/InitOpt/Init/g
#    s/ViewStruct/View/g
#    s/FieldStruct/Field/g
#    s/FormStruct/Form/g
#    s/PickStruct/Pick/g
#    s/MenuStruct/Menu/g
