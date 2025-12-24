:                APPLICATIONS
:     Neovim
!exec nvim
:     Root Neovim
!exec rsh -c nvim
:     Full Screen (root) Shell
!exec rsh
:     Test Curses Keys
!ckeys
:     View a Source File
!pick -i ./ls -n 1 -c bat
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -c iloan -o iloan.dat
:     Listener Research
!form listadd.d
:     Cash Receipts
!form receipt.f receipt.sh
:     Form Data Types
!form -d fields.f -i fields.dat -o fields.dat
:     Edit .c Files in Current Directory
!pick -S "lf ./ .*\.c$" -n 1 -c vi -T "Select File to Edit"
:     View Data Types Help File
!view -L 17 -C 45 -T "Data Types" /home/bill/menuapp/data/fields.f
:     Menu Description
!view -L 36 -C 74 main.m
:     Exit Applications
!return
