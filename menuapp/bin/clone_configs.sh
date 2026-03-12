#!/bin/bash

sed 's/ghostty/kitty/g' ghostty_fonts.sh >kitty_fonts.sh
sed 's/ghostty/kitty/g' ghostty_chfont.sh >kitty_chfont.sh
sed 's/ghostty/kitty/g' ghostty_themes.sh >kitty_themes.sh
sed 's/ghostty/kitty/g' ghostty_chtheme.sh >kitty_chtheme.sh
chmod a+x kitty_*.sh
cp kitty_*.sh ~/.local/bin/
sed 's/ghostty/alacritty/g' ghostty_fonts.sh >alacritty_fonts.sh
sed 's/ghostty/alacritty/g' ghostty_chfont.sh >alacritty_chfont.sh
sed 's/ghostty/alacritty/g' ghostty_themes.sh >alacritty_themes.sh
sed 's/ghostty/alacritty/g' ghostty_chtheme.sh >alacritty_chtheme.sh
chmod a+x alacritty_*.sh
cp alacritty_*.sh ~/.local/bin/
