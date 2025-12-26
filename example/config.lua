bind_key("Mod+F1", function() print("mod + f1 pressed") end)


print(opt.keyboard.layout)

opt.keyboard.layout = "de"
opt.keyboard.rule = "evdev"
opt.keyboard.model = "pc105"
opt.keyboard.variant = ""
opt.keyboard.options = "grp:alt_shift_toggle"
