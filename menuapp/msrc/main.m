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
!form iloan.f -i iloan.dat -c iloan -o iloan.dat
:     Listener Research
!form listadd.f -i listadd.dat -o listadd.dat
:     Cash Receipts
!form receipt.f -i receipt.dat -o receipt.dat
:     Delete File by Inode
!pick -S "ls -i" -n 1 -R "rm -i"
:     Form Data Types
!form -d fields.f -i fields.dat -o fields.dat
:     Edit .c Files in Current Directory
!pick -S "lf ./ .*\.c$" -n 1 -R vi -T "Select File to Edit"
:     View a C File
!view -L 50 -C 80 -S "tree-sitter highlight /home/bill/menuapp/data/init_view.c"
:     View Data Types Help File
!view -L 17 -C 45 -T "Data Types" /home/bill/menuapp/data/fields.f
:     Menu Description
!view -L 30 -C 78 -S "bat -l Crystal -f /home/bill/menuapp/msrc/main.m"
:     Help
!view view -L 30 -C 78 -S "optsp.sh"
:     Exit Applications
!return
