# Neovim Configuration

## Custom Highlights

To use custom_highlights.lua, you can copy it to your nvim config directory. For example, if your nvim config directory is located at ~/.config/nvim, you can run the following command:

```bash
    cp custom_highlights.lua ~/.config/nvim/lua/config
```

and add the following line to your init.lua file:

```lua
require("config.custom_highlights")
```

## Colorizer

[Norcalli](https://github.com/norcalli)

[Norcalli nvim-colorizer Repo](https://github.com/norcalli/nvim-colorizer.lua)

AI Overview from [ChatGPT](https://chat.openai.com/)

The GitHub profile for the user "Norcalli" (Ashkan Kiani), a core developer on the Neovim project, can be found at [Norcalli](https://github.com/norcalli).

They have developed several popular Neovim plugins in Lua, including:

[nvim-colorizer.lua]: A high-performance, dependency-free plugin that highlights color codes in your buffer.

[nvim.lua](https://github.com/norcalli/nvim.lua): A Lua module providing useful shortcut/magic methods for mappings.

[profiler.nvim](https://github.com/norcalli/profiler.nvim): A profiling tool for Neovim.

typeracer.nvim: A Neovim game to practice typing speed.

[nvim-terminal.lua](https://github.com/norcalli/nvim-terminal.lua): A high performance filetype mode for Neovim leveraging conceal and highlights.

[nvim-base16.lua](https://github.com/norcalli/nvim-base16.lua): A programmatic Lua library for setting base16 themes in Neovim.

[neovim-plugin](https://github.com/norcalli/neovim-plugin): A Lua library to help standardize the creation of Lua-based Neovim plugins.

---

Add colorizer.lua containing the following code to lua/plugins directory:

```lua
return {
	{
		"norcalli/nvim-colorizer.lua",
		config = function()
			require("colorizer").setup({
				"*",
			})
		end,
	},
}
```

## Snacks.nvim and a Slick Neovim Logo

[Folke](https://github.com/folke)

[Folke LazyVim Repo](https://github.com/LazyVim/LazyVim)

[Folke Snacks Repo](https://github.com/folke/snacks.nvim)

AI Overview from [ChatGPT](https://chat.openai.com/)

Folke Lemaitre, known as "folke" on GitHub, is a prominent developer in the Neovim community, most recognized as the author of the popular plugin manager lazy.nvim and the comprehensive Neovim configuration framework LazyVim.

He has developed a suite of widely used Neovim plugins, including:

[lazy.nvim](https://github.com/folke/lazy.nvim): A modern plugin manager known for its powerful UI, automatic caching, bytecode compilation for fast startup times, and focus on easy configuration.

[LazyVim](http://www.lazyvim.org/): A full-featured Neovim setup powered by lazy.nvim, designed to be easily customizable and extended.

[which-key.nvim](https://github.com/folke/snacks.nvim): A plugin that helps users remember keymaps by showing available keybindings in a popup as they type.

[noice.nvim](https://github.com/folke/neodev.nvim): A highly experimental plugin that completely replaces the UI for messages, the command line, and the popup menu.

[trouble.nvim](https://github.com/folke/drop.nvim) A utility for pretty display of diagnostics, references, quickfix lists, and more to help manage code issues.

[flsh.nvim](https://github.com/folke/flash.nvim): A plugin that enhances code navigation with search labels, improved character motions, and Treesitter integration.

[tokyonight.nvim](https://github.com/folke/ts-comments.nvim): A plugin that provides a distraction-free coding environment by focusing on the active window.

Folke's contributions are significant in shaping the modern Neovim experience, particularly by leveraging Lua for more powerful and efficient configurations and plugins. Many of his projects can be explored on his
[GitHub profile](https://github.com/folke) or the [Dotfyle](https://dotfyle.com/folke) community page.

---

Snacks is a Neovim plugin that provides a collection of useful snippets and utilities for Neovim users. One of the features of Snacks is the ability to display a slick Neovim logo in the splash screen when Neovim starts up. The Neovim logo is an ASCII Art image which is included in this distribution under configs/nvim/lua/plugins/snacks.lua.

I don't know who created this particular Neovim Unicode Art, but it has been shared widely in the Neovim community and is often used as a splash screen for Neovim configurations. It is possible that the original creator of this logo is unknown or that it was created by multiple people over time. One clue is that it exclusively uses the Unicode code points listed below:

```unicode
    █  2588
      E0BA
      E0B8
      E0BE
      E0BC
```

```lua
[[
      ████ ██████           █████      ██
     ███████████             █████ 
     █████████ ███████████████████ ███   ███████████
    █████████  ███    █████████████ █████ ██████████████
   █████████ ██████████ █████████ █████ █████ ████ █████
 ███████████ ███    ███ █████████ █████ █████ ████ █████
██████  █████████████████████ ████ █████ █████ ████ ██████]],
```

You may have noticed that all but the first code points are in the Private Use Area of Unicode, which means they are not assigned to any specific character and can be used for custom purposes. This is a common practice in ASCII Art and Unicode Art to create unique designs without relying on standard characters. If anyone knows the original creator of this logo, please let me know so I can give them proper credit.

---
