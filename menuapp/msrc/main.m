:                APPLICATIONS
:
:
:      Yast
!exec rsh -c ~/menuapp/bin/yast
:      Full Screen (root) Shell
!exec rsh
:       Test Curses Keys
!ckeys
:       Write Configuration File
!write_config
:       Install Configuration File
!~/menuapp/user/inst_config >~/menuapp/user/shell_msg
:       Pick Edit a File
!pick -i picklist -M -c vi picklist.out
:       Cash Receipts
!form receipts.f receipt.sh
:
:      Help
!help ~/menuapp/doc/applications.hlp
:
:      Exit Applications
!return
