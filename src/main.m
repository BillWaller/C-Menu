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
!pick -i picklist -c vi picklist.out
:       Test Forms
!form form1.d
:       Contact Information
!form contact.d contact.sh
:      Display Banner
!form banner.d
:
:      Help
!help
:
:      Exit Applications
!return
