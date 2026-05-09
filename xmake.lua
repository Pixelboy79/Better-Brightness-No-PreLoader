add_rules("mode.debug", "mode.release")

target("BetterBrightness")
    set_kind("shared")
    add_files("src/*.cpp")
    
    -- Android specifics
    if is_plat("android") then
        add_syslinks("log")
        add_linkdirs("lib")
        add_links("GlossHook") -- Link the GlossHook static library
end
