local current_dir = _SCRIPT_DIR

filter({})
sysincludedirs(current_dir .. "/public")
libdirs({current_dir .. "/lib"})
links("steam_api")