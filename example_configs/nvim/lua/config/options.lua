-- in init.lua, add
-- require("config.options")
-- to turn this options.lua off, uncomment the following line
-- if true then return {} end
vim.g.mapleader = " "
vim.g.maplocalleader = "\\"
vim.g.have_nerd_font = true
vim.g.editorconfig = false
vim.g.lazyvim_mini_snippets_in_completion = true
local opt = vim.opt
Stdbg = "#000000"
opt.shiftwidth = 4
opt.tabstop = 4
opt.softtabstop = 4
opt.smarttab = false
opt.autoindent = false
opt.smartindent = false
opt.breakindent = true
-- opt.smartcase = true
opt.ignorecase = false
opt.number = true
opt.relativenumber = true
opt.wrap = true
opt.swapfile = false
opt.undofile = true
opt.splitright = true
opt.splitbelow = true
opt.list = true
-- opt.cmdheight = 0
opt.termguicolors = true
vim.api.nvim_command([[
    augroup ChangeBackgroudColour
        autocmd colorscheme * :hi normal guibg=Stdbg
    augroup END
]])
opt.mouse = "a"
opt.clipboard = "unnamedplus"
opt.updatetime = 300
opt.timeoutlen = vim.g.vscode and 1000 or 300
vim.o.winborder = "rounded"
vim.g.ai_cmp = false
vim.o.tags = "./tags;.,tags;"
vim.bo.textwidth = 80
vim.bo.formatoptions = "tcqawj"
vim.opt.termsync = false
