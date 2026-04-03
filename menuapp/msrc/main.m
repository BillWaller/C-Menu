:                APPLICATIONS
#
:     Neovim
!exec nvim
:     Root Neovim
!exec rsh -c nvim
:     Full Screen (root) Shell
!exec rsh
:   Workstation Configuration
!menu workstation_config.m
:   Diagnostic Tools
!menu diag.m
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -o iloan.dat
:     Listener Research
!form listadd.f -i listadd.dat -o listadd.dat
:     Cash Receipts
!form receipt.f -i receipt.dat -o receipt.dat
:     Form Data Types
!form -d fields.f -i fields.dat -o fields.dat
:     Edit .c Files in Current Directory
!pick -S project_src -c nvim -T "Project Tree - Select File to Edit"
:     View CMenu Source with Tree-Sitter
!pick -S project_src -n 1 -T "Select Project Source to Highlight" -c "view -L 60 -C 70 -S \"tree-sitter highlight %%\""
:     View Source with Tree-Sitter
!pick -S src -n 1 -T "Select Rust File to Highlight" -c "view -L 60 -C 70 -S \"ts_hl.sh %%\""
:     View Data Types Help File
!view -N f -T "Data Types" /home/bill/menuapp/help/fields.hlp
:     Menu Description With Bat Syntax Highlighting
!view -N f -L 30 -C 75 -S "bat --theme ansi -l Crystal -f /home/bill/menuapp/msrc/main.m"
:     View C-Menu Command Line Options
!view -N f -L 66 -C 75 ~/menuapp/help/menu.help
:     View Highlighted view_engine.c
!view -L 66 -C 70 ~/menuapp/help/view_engine.c
:     Exit Applications
!return
