add_rules("mode.debug", "mode.release")

target("sysifus")
  set_kind("shared")
  set_languages("c99")
  set_warnings("all", "error")
  add_files("src/*.c")
  add_headerfiles("include/*.h")
  add_includedirs("include", { public = true })
  set_pcheader("include/luts.h")
  add_cxflags("-Wno-unknown-pragmas")

option("compile_sysifus_tests")
  set_default(false) -- Default to disabled
  set_showmenu(true) -- Show in `xmake f --menu`

target("sysifus_testing")
  if has_config("compile_sysifus_tests") then
    set_kind("binary")
    set_languages("c99")
    set_warnings("all", "error")
    add_files("test/main.c")
    add_deps("sysifus")
    add_includedirs("include")
    add_links("check")
  end
