:SAMPLE MENU
:
:Gnumeric
!exec gnumeric
:Shell Script
!exec bash -c script.sh
:Shell Script as Root
!exec rsh -c script.sh
:Full Screen (root) Shell
!exec rsh
:Test 🎹 Curses Keys
!ckeys
:Play a Sound 🎳 
!play -q /usr/share/sounds/alsa/Front_Center.wav
:Pick Items From a List
!pick -i picklist -M -c vi picklist.out
:Customer Receipts
!form receipts.f -c receipts.sh
:Manual Page Viewer
!nvim +Man!
:
:Help
!help ~/menuapp/doc/applications.hlp
:Exit Applications
!return
