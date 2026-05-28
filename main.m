:                APPLICATIONS
#
:     Full Screen (root) Shell
!exec rsh
:   Workstation Configuration
!menu workstation_config.m
:   -TDiagnostic Tools
!menu diag.m
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -R "view -L60 -C62 -Nf -S \"amort %%\"" -o iloan.dat
:     Listener Research
!form listadd.f -i listadd.dat -o listadd.dat
:     Cash Receipts
!form receipt.f -i receipt.dat -o receipt.dat
:     Form Data Types
!form -d fields.f -i fields.dat -o fields.dat
:     Rustlings Source
!pick -S "lf rustlings -d 5 \"exercises.*\.rs$\"" -n 1 -T "Rustlings Source - Edit" -c nvim.sh %%
:     View Manual Pages
!pick -S "lf ~/menuapp/man" -d 5 -n 1 -T \"Select Manual Page to View\" -c "view %%"
:     Edit .c Files in Current Directory
!pick -S project_src -T "Project Tree - Select File to Edit" -c nvim.sh %%
:     View C-Menu Source with Tree-Sitter
!pick -S project_src -n 1 -T "Select Project Source to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
:     View Source with Tree-Sitter
!pick -S "lf -S -d 5 . \".*\.(rs|c|h|sh|lua|py|cpp|js|html|css)$\"" -n 1 -T "Select Source File to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
:     lf Help
!view -Nf -L50 -C86 ~/menuapp/help/lf.help
:     View Data Types Help File
!view -Nf -L47 -C85 -S "bat --theme ansi -l Crystal -f ~/menuapp/help/fields.hlp"
:     Menu Description With Bat Syntax Highlighting
!view -Nf -L 39 -C 85 -S "bat --theme ansi -l Crystal -f ~/menuapp/msrc/main.m"
:     View C-Menu Command Line Options
!view -Nf -L66 -C75 ~/menuapp/help/menu.help
:     View Highlighted view_engine.c
!view -N -L66 -C85 ~/menuapp/help/view_engine.c
:     Exit Applications
!return
