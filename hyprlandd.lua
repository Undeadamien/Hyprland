-- Minimal test config for always_on_top based on example/hyprland.lua

local terminal = "kitty"

-- Monitor
hl.monitor({
	output = "",
	mode = "preferred",
	position = "auto",
	scale = "auto",
})

hl.bind("SUPER + e", hl.dsp.exec_cmd("rofi -show drun"))

-- Core config
hl.config({
	general = {
		gaps_in = 5,
		gaps_out = 20,
		border_size = 2,
		col = {
			active_border = { colors = { "rgba(33ccffee)", "rgba(00ff99ee)" }, angle = 45 },
			inactive_border = "rgba(595959aa)",
		},
		layout = "dwindle",
	},

	decoration = {
		rounding = 10,
		rounding_power = 2,
		shadow = { enabled = true, range = 4, render_power = 3, color = 0xee1a1a1a },
		blur = { enabled = true, size = 3, passes = 1, vibrancy = 0.1696 },
	},

	animations = { enabled = true },

	input = {
		kb_layout = "us",
		follow_mouse = 1,
		sensitivity = 0,
	},

	misc = {
		force_default_wallpaper = -1,
		disable_hyprland_logo = false,
	},

	dwindle = { preserve_split = true },
})

-- Curves
hl.curve("easeOutQuint", { type = "bezier", points = { { 0.23, 1 }, { 0.32, 1 } } })
hl.curve("easeInOutCubic", { type = "bezier", points = { { 0.65, 0.05 }, { 0.36, 1 } } })
hl.curve("linear", { type = "bezier", points = { { 0, 0 }, { 1, 1 } } })
hl.curve("quick", { type = "bezier", points = { { 0.15, 0 }, { 0.1, 1 } } })

hl.animation({ leaf = "global", enabled = true, speed = 10, bezier = "default" })
hl.animation({ leaf = "border", enabled = true, speed = 5.39, bezier = "easeOutQuint" })
hl.animation({ leaf = "fade", enabled = true, speed = 3.03, bezier = "quick" })

local mainMod = "SUPER"

-- Keybindings
hl.bind(mainMod .. " + Q", hl.dsp.exec_cmd(terminal))
hl.bind(mainMod .. " + C", hl.dsp.window.close())
hl.bind(
	mainMod .. " + M",
	hl.dsp.exec_cmd("command -v hyprshutdown >/dev/null 2>&1 && hyprshutdown || hyprctl dispatch 'hl.dsp.exit()'")
)
hl.bind(mainMod .. " + V", hl.dsp.window.float({ action = "toggle" }))
hl.bind(mainMod .. " + P", hl.dsp.window.pseudo())

-- Focus
hl.bind(mainMod .. " + left", hl.dsp.focus({ direction = "left" }))
hl.bind(mainMod .. " + right", hl.dsp.focus({ direction = "right" }))
hl.bind(mainMod .. " + up", hl.dsp.focus({ direction = "up" }))
hl.bind(mainMod .. " + down", hl.dsp.focus({ direction = "down" }))

-- Workspaces
for i = 1, 10 do
	local key = i % 10
	hl.bind(mainMod .. " + " .. key, hl.dsp.focus({ workspace = i }))
	hl.bind(mainMod .. " + SHIFT + " .. key, hl.dsp.window.move({ workspace = i }))
end

-- Mouse
hl.bind(mainMod .. " + mouse:272", hl.dsp.window.drag(), { mouse = true })
hl.bind(mainMod .. " + mouse:273", hl.dsp.window.resize(), { mouse = true })

-- ======= ALWAYS_ON_TOP TEST BINDS =======
--hl.bind(mainMod .. " + T", hl.dsp.set_prop({ property = "always_on_top", value = "toggle" }))
--hl.bind(mainMod .. " + ALT + T", hl.dsp.set_prop({ property = "always_on_top", value = "on" }))
--hl.bind(mainMod .. " + SHIFT + T", hl.dsp.set_prop({ property = "always_on_top", value = "off" }))

-- ======= WINDOW RULES =======
-- Test: kitty windows start with always_on_top
hl.window_rule({
	name = "aot-kitty",
	match = { class = "kitty" },
	always_on_top = true,
})

-- Test: firefox floating
hl.window_rule({
	name = "float-firefox",
	match = { class = "firefox" },
	float = true,
})

-- Suppress maximize events
hl.window_rule({
	name = "suppress-maximize-events",
	match = { class = ".*" },
	suppress_event = "maximize",
})

-- Fix XWayland drags
hl.window_rule({
	name = "fix-xwayland-drags",
	match = { class = "^$", title = "^$", xwayland = true, float = true },
	no_focus = true,
})

-- Gestures
hl.gesture({ fingers = 3, direction = "horizontal", action = "workspace" })

hl.bind(mainMod .. " + SHIFT + m", hl.dsp.exit())
