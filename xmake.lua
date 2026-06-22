set_languages('cxx17', 'c11')
add_rules("mode.release", "mode.debug", "mode.profile", "mode.check")
set_warnings('all', 'extra')
add_cxflags('-Wno-unused-parameter', {tools = {'gcc', 'clang'}})
add_cxflags('/wd4100', '/wd4201', '/wd4146', '/wd4245', '/wd4127', {tools = {'cl'}})

if is_mode('debug') then
  add_defines('DEBUG')
  set_symbols('debug')
  set_optimize('none')
end

option('sanitize')
  set_default('')
  set_description('enable sanitizers (address, undefined, thread, or comma-separated)')
  set_showmenu(true)
option_end()
local sanitize = get_config('sanitize')
if sanitize and sanitize ~= '' then
  add_cxflags('-fsanitize=' .. sanitize, {force = true})
  add_ldflags('-fsanitize=' .. sanitize, {force = true})
  add_shflags('-fsanitize=' .. sanitize, {force = true})
end

add_defines('NGED_BACKEND_RAYLIB')

add_requires("raylib")
add_requires("spdlog")
add_requires("fmt")
add_requires("nlohmann_json")
add_requires("miniz")
add_requires("doctest")

-- imgui: vendored docking 分支（含 misc/cpp）
target('imgui')
  set_kind('static')
  add_includedirs('deps/imgui', 'deps/imgui/backends', 'deps/imgui/misc/cpp', {public=true})
  add_files('deps/imgui/*.cpp', 'deps/imgui/misc/cpp/*.cpp')
  remove_files('deps/imgui/imgui_demo.cpp')
  if is_plat('windows') then
    add_links('ole32', 'uuid', 'gdi32', 'comctl32', 'dwmapi')
  end

-- rlimgui: vendored，对接项目 docking imgui
target('rlimgui')
  set_kind('static')
  add_deps('imgui')
  add_packages('raylib')
  add_includedirs('deps/rlimgui', {public=true})
  add_files('deps/rlimgui/rlImGui.cpp')

-- nfd
target('nfd')
  set_kind('static')
  add_includedirs('deps/nativefiledialog-extended/src/include', {public=true})
  if is_plat('linux') then
    on_load(function (target)
      import("lib.detect.find_tool")
      local pkgconf = find_tool("pkg-config")
      if pkgconf then
        target:add("cxflags", (os.iorunv(pkgconf.program, {"--cflags", "gtk+-3.0"})):trim())
        target:add("ldflags", (os.iorunv(pkgconf.program, {"--libs", "gtk+-3.0"})):trim())
      end
    end)
    add_files('deps/nativefiledialog-extended/src/nfd_gtk.cpp')
  elseif is_plat('macosx') then
    add_mxflags("-fno-objc-arc")
    add_frameworks("UniformTypeIdentifiers")
    add_files('deps/nativefiledialog-extended/src/nfd_cocoa.m')
  else
    add_files('deps/nativefiledialog-extended/src/nfd_win.cpp')
    add_links('ole32', 'uuid', 'gdi32', 'comctl32')
  end

-- boxer
target('boxer')
  set_kind('static')
  add_includedirs('deps/boxer/include')
  if is_plat('macosx') then
    add_files('deps/boxer/src/boxer_osx.mm')
    add_mxflags("-fno-objc-arc")
  elseif is_plat('linux') then
    add_files('deps/boxer/src/boxer_zenity.cpp')
  else
    add_files('deps/boxer/src/boxer_win.cpp')
  end

-- s7
target('s7')
  add_defines('_GNU_SOURCE', 'sigjmp_buf=jmp_buf')
  set_kind('static')
  add_includedirs('deps/s7', {public=true})
  add_includedirs('examples/ngs7')
  add_files('deps/s7/s7.c', 'examples/ngs7/s7-extensions.cpp')
  if is_plat('linux') then
    add_links('dl')
  end

target('s7e')
  set_kind('binary')
  add_files('examples/ngs7/s7e.cpp')
  add_deps('s7')

-- ngdoc
target('ngdoc')
  set_kind('static')
  add_files('src/ngdoc.cpp', 'src/ngdraw.cpp', 'src/style.cpp')
  add_packages('spdlog', 'miniz', 'fmt', 'nlohmann_json')
  add_includedirs(
    'include',
    'deps/stduuid/include',
    'deps/stduuid',
    'deps/parallel_hashmap/parallel_hashmap',
    {public=true})

-- entry
target('entry')
  set_kind('static')
  add_deps('imgui', 'rlimgui')
  add_packages('raylib')
  add_files('src/entry/entry.cpp', 'src/entry/raylib_main.cpp', 'src/entry/raylib_texture.cpp')
  add_includedirs('include/nged/entry')

-- nged
target('nged')
  set_kind('static')
  add_files('src/nged.cpp', 'src/nged_imgui.cpp', 'src/nged_imgui_fonts.cpp')
  add_deps('nfd', 'imgui', 'boxer', 'ngdoc', 'entry')
  add_packages('spdlog', 'nlohmann_json')
  add_cxflags('/bigobj', {tools='cl'})
  add_includedirs(
    'include',
    'deps/boxer/include',
    'deps/parallel_hashmap/parallel_hashmap')

-- tests
target('tests')
  set_kind('binary')
  add_deps('ngdoc')
  add_packages('spdlog', 'doctest')
  add_files('tests/*.cpp')
  add_includedirs('.')

-- demos
target('demo')
  set_kind('binary')
  add_deps('nged', 'entry')
  add_packages('spdlog')
  add_files('examples/demo/main.cpp')
  add_includedirs('.', 'deps/boxer/include')

target('typed_demo')
  set_kind('binary')
  add_deps('nged', 'entry')
  add_packages('spdlog')
  add_files('examples/typed_demo/main.cpp')
  add_includedirs('.', 'deps/boxer/include')

target('ngs7')
  set_kind('binary')
  add_deps('nged', 's7', 'entry')
  add_packages('spdlog', 'nlohmann_json')
  add_files('examples/ngs7/ngs7.cpp', 'examples/ngs7/main.cpp')
  add_includedirs('.', 'deps/boxer/include', 'deps/subprocess.h')
