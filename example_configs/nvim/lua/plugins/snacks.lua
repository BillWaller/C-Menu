return {
	"folke/snacks.nvim",
	priority = 1000,
	lazy = false,
	opts = {
		image = { enabled = true },
		bigfile = { enabled = true },
		explorer = { enabled = true },
		indent = { enabled = false },
		input = { enabled = true },
		picker = { enabled = true },
		notifier = {
			enabled = true,
		},
		quickfile = { enabled = true },
		scope = { enabled = true },
		scroll = { enabled = true },
		statuscolumn = { enabled = true },
		words = { enabled = true },
		rename = { enabled = false },
		zen = { enabled = false },
		---@class snacks.dashboard.Config
		-- ---@field sections snacks.dashboard.Section
		dashboard = {
			enabled = true,
			preset = {
				keys = {
					-- 📂
					-- 📝
					-- 🔍
					-- ⚙️
					-- ❌
					-- 🛠️
					{ icon = "🔍 ", key = "f", desc = "Find File", action = ":lua Snacks.dashboard.pick('files')" },
					{ icon = "🆕 ", key = "n", desc = "New File", action = ":ene | startinsert" },
					{
						icon = "🔎 ",
						key = "g",
						desc = "Find Text",
						action = ":lua Snacks.dashboard.pick('live_grep')",
					},
					{
						icon = "📤 ",
						key = "r",
						desc = "Recent Files",
						action = ":lua Snacks.dashboard.pick('oldfiles')",
					},
					{
						icon = "⚙️ ",
						key = "c",
						desc = "Config",
						action = ":lua Snacks.dashboard.pick('files', {cwd = vim.fn.stdpath('config')})",
					},
					{ icon = "🔙 ", key = "s", desc = "Restore Session", section = "session" },
					{ icon = "📦 ", key = "e", desc = "Lazy Extras", action = ":LazyExtras" },
					{ icon = "💤 ", key = "l", desc = "Lazy", action = ":Lazy" },
					{
						icon = "🛠️ ",
						key = "m",
						desc = "Mason",
						action = ":Mason",
						enabled = package.loaded.lazy ~= nil,
					},
					{
						icon = "🌳 ",
						key = "t",
						desc = "Tree Sitter",
						action = ":TSUpdate",
						enabled = package.loaded.lazy ~= nil,
					},
					{
						icon = "✅ ",
						key = "h",
						desc = "Check Health",
						action = ":checkhealth",
						enabled = package.loaded.lazy ~= nil,
					},
					{
						icon = "🛑 ",
						key = "q",
						desc = "Quit - Close All Files and Exit NVIM",
						action = ":qa",
					},
				},
				---@type fun(cmd:string, opts:table)|nil
				pick = nil,
				-- ---@type snacks.dashboard.Item[]
				header = [[      ████ ██████           █████      ██                    
     ███████████             █████                            
     █████████ ███████████████████ ███   ███████████  
    █████████  ███    █████████████ █████ ██████████████  
   █████████ ██████████ █████████ █████ █████ ████ █████  
 ███████████ ███    ███ █████████ █████ █████ ████ █████ 
██████  █████████████████████ ████ █████ █████ ████ ██████]],
				--
			},
			sections = {
				{
					section = "header",
					align = "center",
					width = 70,
					height = 6,
				},
				{
					section = "keys",
					padding = 1,
				},
				{ section = "recent_files", icon = " ", title = "Recent Files", indent = 3, padding = 1 },
				{ section = "startup" },
			},
		},
	},
}
