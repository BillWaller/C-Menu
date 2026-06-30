# C-Menu Sample Application Menu


```
:   -RFull Screen (Root) Shell
!exec rsh
```


:   -RFull Screen (Root) Shell
!exec rsh
:     Youtube (in Firefox)
!dexe firefox https://www.youtube.com
:     C-Menu (in ghostty)
!dexe ghostty -e menu
:     Kitty HTOP
!exec kitty --detach -o initial_window_width=80c -o initial_window_height=20c htop
:     Ghostty HTOP
!dexe ghostty --window-width=80 --window-height=20 -e htop
:     Issue RSH Certificate
!form rshusers.f -i rshusers.dat -o rshusers.dat
:   Workstation Configuration
!menu workstation_config.m
:   Diagnostic Utilities
!menu diag.m
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -R "view -L60 -C62 -Nf -S \"amort %%\"" -o iloan.dat
:     Cash Receipts
!form receipt.f -i receipt.dat -o receipt.dat
:     Rustlings Source
!pick -S "lf -S rustlings -d3 \".*exercises.*\.rs$\"" -v -n 1 -T "Rustlings Source - Edit" -c "nvim %%"
:     -PView Manual Pages
!pick -S "listman.sh" -n 1 -T \"Select Manual Page to View\" -c "readman.sh %%"
:     Edit C-Menu Description Files
!pick -S list_msrc -n1 -T "C-Menu Description Files - Select File to Edit" -c edit_msrc %%
:     -SView C-Menu Source with Tree-Sitter
!pick -S project_src -n 1 -T "Select Project Source to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
:     -TView Source with Tree-Sitter
!pick -S "lf -S -d 5 . \".*\.(rs|c|h|sh|lua|py|cpp|js|html|css)$\"" -T "Select Source File to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
:   View LSP Log
!view -L60 -C80 /home/bill/.local/state/nvim/logs/lsp.log
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
