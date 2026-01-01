:                APPLICATIONS
:     Neovim
!exec nvim
:     Root Neovim
!exec rsh -c nvim
:     Full Screen (root) Shell
!exec rsh
:     Test Curses Keys
!ckeys
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -o iloan.dat
:     Listener Research
!form listadd.f -i listadd.dat -o listadd.dat
:     Cash Receipts
!form receipt.f -i receipt.dat -o receipt.dat
:     Form Data Types
!form -d fields.f -i fields.dat -o fields.dat
:     Edit .c Files in Project Tree
!pick -S "lf -r ./ .*\.[ch]$" -M -c vi -T "Project Tree - Select File to Edit"
:     View Tree-Sitter Highlighted .c Files
!pick -S "lf -r ./ .*\.[ch]$" -n 1 -c "view -S \"tree-sitter highlight %%\"" -T "Project Tree - Select File to Edit"
:     View Data Types Help File
!view -L 40 -C 80 -T "Data Types" /home/bill/menuapp/help/fields.hlp
:     Menu Description
!view -L 30 -C 78 -S "bat -l Crystal -f /home/bill/menuapp/msrc/main.m"
:     Help
!view view -L 30 -C 78 -S "optsp.sh"
:     Exit Applications
!return
