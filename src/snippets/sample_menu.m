:   SAMPLE MENU
#   /home/bill/menuapp/msrc/main.m
#   This is the main menu for this application.
:   Full Screen (root) Shell
!exec rsh
:   Test Curses Keys
!ckeys
:   Installment Loan Calculations (Form)
!form iloan.f -i iloan.dat -S iloan -o iloan.dat
:   Form Data Types (Form)
!form -d fields.f -i fields.dat -o fields.dat
:   Edit .c Files in Current Directory (Pick)
!pick -S project_src -c nvim -T "Project Tree - Select File to Edit"
:   View CMenu Source with Tree-Sitter (Pick, View)
!pick -S project_src -n 1 -T "Select Project File to Highlight" -c "view -L 60 -C 70 -S \"tree-sitter highlight %%\""
:   Edit Menu Description
!exec vi ~/menuapp/msrc/main.m
:   Help
!help
:   Exit Applications
!return
