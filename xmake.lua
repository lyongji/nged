set_languages('cxx17', 'c11')
add_rules("mode.release")
add_rules("mode.debug")
add_rules("mode.profile")
add_rules("mode.check")
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

option('backend')
  if is_host('windows') then
    set_default('dx11')
  elseif is_host('macosx') then
    set_default('metal')
  else
    set_default('gl2')
  end
  set_description('renderer backend, default to dx11 on windows, metal on macos, otherwise gl2')
  set_showmenu(true)
  set_values('dx11', 'dx12', 'vulkan', 'gl2', 'gl3', 'metal', 'raylib')
option_end()
local backend = get_config('backend')
if is_plat('windows') and (backend=='vulkan' or backend=='gl2' or backend=='gl3') then
  add_requires('vcpkg::glfw3')
elseif is_plat('macosx') then
  brew = os.getenv('HOMEBREW_PREFIX')
  if brew then
    add_includedirs(path.join(brew, 'include'))
    add_linkdirs(path.join(brew, 'lib'))
  end
end
if backend=='raylib' then
  add_requires('raylib')
  add_defines('NGED_BACKEND_RAYLIB')
elseif backend=='dx11' then
  add_defines('NGED_BACKEND_DX11')
elseif backend=='dx12' then
  add_defines('NGED_BACKEND_DX12')
elseif backend=='vulkan' then
  add_defines('NGED_BACKEND_VULKAN')
elseif backend=='gl2' then
  add_defines('NGED_BACKEND_GL2')
elseif backend=='gl3' then
  add_defines('NGED_BACKEND_GL3')
elseif backend=='metal' then
  add_defines('NGED_BACKEND_METAL')
end

add_requires("spdlog")
add_requires("fmt")
add_requires("nlohmann_json")
add_requires("miniz")
add_requires("doctest")

option('vulkan-sdk')
  set_description('vulkan sdk path')
  set_showmenu(true)
  set_default('')
local vulkan_sdk = get_config('vulkan-sdk')

-- imgui（vendored: docking 分支 + Metal patch + misc/cpp）
target('imgui')
  set_kind('static')
  add_includedirs('deps/imgui', 'deps/imgui/backends', 'deps/imgui/misc/cpp', {public=true})
  add_headerfiles('deps/imgui/*.h', 'deps/imgui/misc/cpp/*.h')
  add_files('deps/imgui/*.cpp', 'deps/imgui/misc/cpp/*.cpp')
  remove_files('deps/imgui/imgui_demo.cpp')
  if backend=='dx11' then
    add_files('deps/imgui/backends/imgui_impl_win32.cpp', 'deps/imgui/backends/imgui_impl_dx11.cpp')
    add_links('d3d11', 'dxgi', 'd3dcompiler')
  elseif backend=='dx12' then
    add_files('deps/imgui/backends/imgui_impl_win32.cpp', 'deps/imgui/backends/imgui_impl_dx12.cpp')
    add_links('d3d12', 'dxgi', 'd3dcompiler')
  elseif backend=='vulkan' then
    add_files('deps/imgui/backends/imgui_impl_glfw.cpp', 'deps/imgui/backends/imgui_impl_vulkan.cpp')
    add_includedirs(vulkan_sdk..'/Include', {public=true})
    add_linkdirs(vulkan_sdk..'/Lib', {public=true})
    add_links('glfw3')
    if is_plat('windows') then
      add_links('vulkan-1')
    else
      add_links('vulkan')
    end
  elseif backend=='gl2' then
    add_files('deps/imgui/backends/imgui_impl_glfw.cpp', 'deps/imgui/backends/imgui_impl_opengl2.cpp')
  elseif backend=='gl3' then
    add_files('deps/imgui/backends/imgui_impl_glfw.cpp', 'deps/imgui/backends/imgui_impl_opengl3.cpp')
  elseif backend=='metal' then
    add_files('deps/imgui/backends/imgui_impl_glfw.cpp')
    on_load(function (target)
      local outdir = path.join(target:autogendir(), "imgui_patched")
      local patched = path.join(outdir, "imgui_impl_metal.mm")
      target:add("files", patched)
    end)
    before_build(function (target)
      import("lib.detect.find_tool")
      local patch_tool = find_tool("patch")
      if not patch_tool then
        raise("patch program not found, required for Metal backend")
      end
      local orig = path.join(os.projectdir(), "deps/imgui/backends/imgui_impl_metal.mm")
      local patchfile = path.join(os.projectdir(), "patches/imgui_impl_metal.patch")
      local outdir = path.join(target:autogendir(), "imgui_patched")
      local patched = path.join(outdir, "imgui_impl_metal.mm")
      os.mkdir(outdir)
      os.cp(orig, patched)
      os.runv(patch_tool.program, {"-u", "-N", "-p2", "-i", patchfile, "-d", outdir}, {try = true})
    end)
  elseif backend=='raylib' then
    -- rlImGui (vendored) handles backend integration
  end
  if is_plat('windows') or is_plat('msys') or is_plat('mingw') then
    add_links('ole32', 'uuid', 'gdi32', 'comctl32', 'dwmapi')
    if backend=='gl2' or backend=='gl3' then
      add_links('glfw3', 'opengl32')
      add_packages('vcpkg::glfw3')
    elseif backend=='vulkan' then
      add_packages('vcpkg::glfw3')
    end
  elseif backend~='raylib' then
    add_links('glfw', 'dl', 'pthread')
    if is_plat('macosx') then
      if backend=='metal' then
        add_frameworks('Metal', 'QuartzCore')
      else
        add_frameworks('OpenGL')
      end
    else
      add_links('GL')
    end
  end

-- rlimgui（vendored，对接项目 docking imgui）
if backend=='raylib' then
  target('rlimgui')
    set_kind('static')
    add_deps('imgui')
    add_packages('raylib')
    add_includedirs('deps/rlimgui', 'deps/imgui', 'deps/imgui/backends', {public=true})
    add_files('deps/rlimgui/rlImGui.cpp')
end

-- nfd（vendored）
target('nfd')
  set_kind('static')
  add_includedirs('deps/nativefiledialog-extended/src/include', {public=true})
  add_headerfiles('deps/nativefiledialog-extended/src/include/*.h')
  local nfdimp = 'nfd_win.cpp'
  if is_plat('macosx') then
    nfdimp = 'nfd_cocoa.m'
    add_mxflags("-fno-objc-arc")
    add_frameworks("UniformTypeIdentifiers")
  elseif is_plat('linux') then
    nfdimp = 'nfd_gtk.cpp'
    on_load(function (target)
      import("lib.detect.find_tool")
      local pkgconf = find_tool("pkg-config")
      if pkgconf then
        target:add("cxflags", (os.iorunv(pkgconf.program, {"--cflags", "gtk+-3.0"})):trim())
        target:add("ldflags", (os.iorunv(pkgconf.program, {"--libs", "gtk+-3.0"})):trim())
      end
    end)
  end
  add_files('deps/nativefiledialog-extended/src/'..nfdimp)
  if is_plat('windows') or is_plat('msys') then
    add_links('ole32', 'uuid', 'gdi32', 'comctl32')
  end

-- boxer（vendored）
target('boxer')
  set_kind('static')
  add_includedirs('deps/boxer/include')
  add_headerfiles('deps/boxer/include/**')
  if is_plat('macosx') then
    add_files('deps/boxer/src/boxer_osx.mm')
    add_mxflags("-fno-objc-arc")
  elseif is_plat('linux') then
    add_files('deps/boxer/src/boxer_zenity.cpp')
  else
    add_files('deps/boxer/src/boxer_win.cpp')
  end

-- s7（vendored）
target('s7')
  set_kind('static')
  add_includedirs('deps/s7', {public=true})
  add_includedirs('examples/ngs7')
  add_headerfiles('deps/s7/s7.h', 'examples/ngs7/s7-extensions.h')
  add_files      ('deps/s7/s7.c', 'examples/ngs7/s7-extensions.cpp')
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
  add_headerfiles('include/nged/ngdoc.h')
  add_files('src/ngdoc.cpp', 'src/ngdraw.cpp', 'src/style.cpp')
  add_packages('spdlog', 'miniz', 'fmt', 'nlohmann_json')
  add_includedirs(
    'include',
    'deps/stduuid/include',
    'deps/stduuid', -- for gsl
    'deps/parallel_hashmap/parallel_hashmap',
    {public=true})

-- entry
target('entry')
  set_kind('static')
  add_deps('imgui')
  if backend=='raylib' then
    add_deps('rlimgui')
    add_packages('raylib')
  end
  add_includedirs('include/nged/entry')
  add_files('src/entry/entry.cpp')
  if backend=='dx11' then
    add_files('src/entry/dx11_main.cpp')
    add_files('src/entry/dx11_texture.cpp')
  elseif backend=='dx12' then
    add_files('src/entry/dx12_main.cpp')
    add_files('src/entry/dx12_texture.cpp')
  elseif backend=='vulkan' then
    add_files('src/entry/vulkan_main.cpp')
    add_files('src/entry/vulkan_texture.cpp')
  elseif backend=='gl2' then
    add_files('src/entry/gl2_main.cpp')
    add_files('src/entry/gl_texture.cpp')
  elseif backend=='gl3' then
    add_files('src/entry/gl3_main.cpp')
    add_files('src/entry/gl_texture.cpp')
  elseif backend=='metal' then
    add_files('src/entry/metal_main.mm')
    add_files('src/entry/metal_texture.mm')
  elseif backend=='raylib' then
    add_files('src/entry/raylib_main.cpp')
    add_files('src/entry/raylib_texture.cpp')
  end
  if is_plat('windows') then
    add_links('ws2_32', 'advapi32', 'gdi32', 'shell32', 'version')
    if backend~='dx11' and backend~='dx12' then
      add_packages('vcpkg::glfw3')
    end
    if backend=='gl2' or backend=='gl3' then
      add_packages('vcpkg::gl3w')
    end
  elseif is_plat('macosx') then
    add_links('iconv')
  end

  if is_plat('macosx') then
    add_frameworks('AppKit', 'IOKit', 'QuartzCore')
    if backend=='metal' then
      add_frameworks('Metal', 'Foundation')
    end
  end

-- nged
target('nged')
  set_kind('static')
  add_headerfiles('include/nged/*.h|ngdoc.h')
  add_files('src/nged.cpp', 'src/nged_imgui.cpp', 'src/nged_imgui_fonts.cpp')
  add_deps('nfd', 'imgui', 'boxer', 'ngdoc', 'entry')
  add_packages('spdlog', 'nlohmann_json')
  add_cxflags('/bigobj', {tools='cl'})
  add_includedirs(
    'include',
    'deps/boxer/include',
    'deps/parallel_hashmap/parallel_hashmap'
  )

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
  add_includedirs('deps/imgui/backends')
  if is_plat('windows') then
    add_files('examples/ngs7/icon.rc')
  end
  add_includedirs('.', 'deps/boxer/include', 'deps/imgui', 'deps/subprocess.h')
