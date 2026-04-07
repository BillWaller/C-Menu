:   WORKSTATION CONFIGURATION
:
:   View SDDM Previews
!exec sddm_preview.sh
:   Select SDDM Background
!pick -n 1 -T "Select SDDM Background" -S sddm_bg.sh -c "sddm_chbg.sh %%"
:   Select Ghostty Font
!pick -n 1 -T "Select Ghostty Font" -S ghostty_fonts.sh -c "ghostty_chfont.sh %%"
:   Select Ghostty Theme
!pick -n 1 -T "Select Ghostty Theme" -S ghostty_themes.sh -c "ghostty_chtheme.sh %%"
:   Select Kitty Font
!pick -n 1 -T "Select Kitty Font" -S kitty_fonts.sh -c "kitty_chfont.sh %%"
:   Select Kitty Theme
!pick -n 1 -T "Select Kitty Theme" -S kitty_themes.sh -c "kitty_chtheme.sh %%"
:   Select Alacritty Font
!pick -n 1 -T "Select Alacritty Font" -S alacritty_fonts.sh -c "alacritty_chfont.sh %%"
:   Select Alacritty Theme
!pick -n 1 -T "Select Alacritty Theme" -S alacritty_themes.sh -c "alacritty_chtheme.sh %%"
:     Help
!help
:     Exit Workstation Configuration
!return
