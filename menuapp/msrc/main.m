:    MAIN MENU
#
:   -RFull Screen (Root) Shell
!exec rsh
:   Workstation Configuration
!menu workstation_config.m
:   Diagnostic Utilities
!menu diag.m
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -R "view -L60 -C62 -Nf -S \"amort %%\"" -o iloan.dat
:     Doxygen
!dexe doxywizard /srv/www/htdocs/C-Menu/Doxyfile
:     RSH Time Based Certificates
!form rshusers.f -i rshusers.dat -o rshusers.dat
:     Cash Receipts
!form receipt.f -i receipt.dat -o receipt.dat
:     Rustlings Source
!pick -S "lf rustlings -d 5 \"exercises.*\.rs$\"" -n 1 -T "Rustlings Source - Edit" -c nvim.sh %%
:     -PView Manual Pages
!pick -S "lf ~/menuapp/man" -d 5 -n 1 -T \"Select Manual Page to View\" -c "view -Nf %%"
:     Edit C-Menu Description Files
!pick -S list_msrc -n 1 -T "C-Menu Description Files - Select File to Edit" -c edit_msrc %%
:     -SView C-Menu Source with Tree-Sitter
!pick -S project_src -n 1 -T "Select Project Source to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
:     -TView Source with Tree-Sitter
!pick -S "lf -S -d 5 . \".*\.(rs|c|h|sh|lua|py|cpp|js|html|css)$\"" -n 1 -T "Select Source File to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
:     Help Menu
!menu help.m
:     -BMenu Description With Bat Syntax Highlighting
!view -Nf -L 39 -C 85 -S "bat --theme ansi -l Crystal -f ~/menuapp/msrc/main.m"
:     -OView C-Menu Command Line Options
!view -Nf -L66 -C75 ~/menuapp/help/menu.help
:     -eView Highlighted view_engine.c
!view -N -L66 -C85 ~/menuapp/help/view_engine.c
:     Exit Applications
!return
