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
  set_values('dx11', 'dx12', 'vulkan', 'gl2', 'gl3', 'metal')
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
if backend=='dx11' then
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

option('python')
  set_description('python executable path, or "no" if python binding is not needed, or "auto"')
  set_showmenu(true)
  set_default('auto')
option_end()

option('pyextension_fullpath')
  set_default('')
  set_showmenu(true)
option_end()

rule('parm_bin2c')
  set_extensions('.lua')
  on_load(function (target) 
    target:add('includedirs', target:autogendir())
  end)
  on_build_file(function (target, sourcefile, opt)
    import("core.project.depend")
    import("lib.detect.find_tool")
    local python = get_config("python")
    if not python or python == "auto" or python == "no" then
      local tool = find_tool("python3") or find_tool("python")
      if tool then python = tool.program end
    end
    if not python then python = "python3" end

    local bin2c = path.join(os.projectdir(), "python/bin2c.py")
    local headerfile = path.join(target:autogendir(), path.filename(sourcefile) .. ".h")
    
    -- ensure output dir exists
    if not os.isdir(path.directory(headerfile)) then
      os.mkdir(path.directory(headerfile))
    end
    
    -- depend.on_changed(function ()
      os.vrunv(python, {bin2c, sourcefile, headerfile})
    -- end, {files = {sourcefile, bin2c}})
  end)
rule_end()

rule('python_config')
  on_load(function (target)
    import("lib.detect.find_package")
    import("lib.detect.find_tool")
    
    local pkg = find_package("python", {version = "3"})
    if pkg then
      print("Found python via find_package")
      target:add("includedirs", pkg.includedirs)
      target:add("linkdirs", pkg.linkdirs)
      target:add("links", pkg.links)
      return
    end
    
    print("Warning: Python 3 not found via find_package. Trying manual detection...")
    import("lib.detect.find_tool")
    local python = get_config("python")
    if not python or python == "auto" or python == "no" then
      local tool = find_tool("python3") or find_tool("python")
      if tool then python = tool.program end
    end
    if not python then python = "python3" end
    
    if python then
      print("Using python executable: " .. python)
      try
      {
        function()
          local inc = os.iorunv(python, {"-c", "import sysconfig; print(sysconfig.get_path('include'), end='')"})
          if inc then
            inc = inc:trim()
            print("Python include: " .. inc)
            target:add("includedirs", inc)
          end
          if is_plat("windows") then
            local libdir = os.iorunv(python, {"-c", "import sysconfig; print(sysconfig.get_config_var('LIBDIR'), end='')"})
            if libdir then target:add("linkdirs", libdir:trim()) end
            print("Python libdir: " .. libdir)
          end
        end
      }
      catch
      {
        function(e)
          print("Error detecting python paths: " .. tostring(e))
        end
      }
    end
  end)
rule_end()

if get_config('python') ~= 'no' then
  target('parmscript')
    set_kind('static')
    add_cxflags('-fPIC', {tools = {'gcc', 'clang'}})
    add_deps('lua', 'imgui', 'nfd')
    add_includedirs('deps/lua', 'deps/sol2/include', 'deps/nlohmann')
    add_includedirs('deps/pybind11/include')
    add_files('deps/parmscript/*.cpp')
    add_files('deps/parmscript/parmexpr.lua', {rule='parm_bin2c'})
    add_rules('python_config')
    on_load(function(target)
        target:add("includedirs", target:autogendir())
    end)

  target('ngpy')
    set_kind('shared')
    add_headerfiles('include/nged/ngpy.h', 'include/nged/pybind11_imgui.h')
    add_files('src/ngpy.cpp', 'src/pybind11_imgui.cpp')
    add_deps('nged', 'entry', 'parmscript', 'lua', 'spdlog')
    add_includedirs('include', 'deps/boxer/include', 'deps/imgui', 'deps/nlohmann', 'deps/parallel_hashmap/parallel_hashmap', 'deps/subprocess.h', 'deps/parmscript')
    add_includedirs('deps/pybind11/include')
    add_cxflags('/bigobj', {tools='cl'})

    add_rules('python_config')

    on_load(function (target)
        import("lib.detect.find_tool")
    local python = get_config("python")
    if not python or python == "auto" or python == "no" then
      local tool = find_tool("python3") or find_tool("python")
      if tool then python = tool.program end
    end
    if not python then python = "python3" end

        local suffix
        try {
            function()
                suffix = os.iorunv(python, {"-c", "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX') or '', end='')"})
            end
        }

        if suffix and #suffix > 0 then
            target:set("filename", "ngpy" .. suffix:trim())
        else
            if is_plat('windows') then
                target:set("filename", "ngpy.pyd")
            else
                target:set("filename", "ngpy.so")
            end
        end

        if is_plat('macosx') then
            target:add("shflags", "-undefined dynamic_lookup")
            -- Assume liblua.a is in the same directory as ngpy.so (or static lib output dir)
            local luapath = path.join(target:targetdir(), "liblua.a")
            print("DEBUG: on_load adding force_load for: " .. luapath)
            target:add("ldflags", "-Wl,-force_load," .. luapath)
            target:add("shflags", "-Wl,-force_load," .. luapath)
        end
    end)

    after_build(function(target)
      local dest = get_config('pyextension_fullpath')
      if dest and dest ~= "" then
        os.cp(target:targetfile(), dest)
      end
    end)
    
    on_run(function(target)
        import("lib.detect.find_tool")
    local python = get_config("python")
    if not python or python == "auto" or python == "no" then
      local tool = find_tool("python3") or find_tool("python")
      if tool then python = tool.program end
    end
    if not python then python = "python3" end

        local envs = {}
        if get_config('pyextension_fullpath') then
             envs.PYTHONPATH = path.directory(get_config('pyextension_fullpath'))
        else
             envs.PYTHONPATH = target:targetdir()
        end
        os.runv(python, {'examples/pydemo/main.py'}, {envs = envs})
    end)
end

option('vulkan-sdk')
  set_description('vulkan sdk path')
  set_showmenu(true)
  set_default('')
local vulkan_sdk = get_config('vulkan-sdk')

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
    if backend=='vulkan' then
      add_includedirs(vulkan_sdk..'/Include', {public=true})
      add_linkdirs(vulkan_sdk..'/Lib', {public=true})
    end
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
  end
  if is_plat('windows') or is_plat('msys') or is_plat('mingw') then
    add_links('ole32', 'uuid', 'gdi32', 'comctl32', 'dwmapi')
    if backend=='gl2' or backend=='gl3' then
      add_links('glfw3', 'opengl32')
      add_packages('vcpkg::glfw3')
    elseif backend=='vulkan' then
      add_packages('vcpkg::glfw3')
    end
  else
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

target('spdlog')
  set_kind('static')
  add_defines('SPDLOG_FMT_EXTERNAL=1', {public=true})
  add_defines('SPDLOG_COMPILED_LIB=1', {public=true})
  add_includedirs('deps/spdlog/include', {public=true})
  add_headerfiles('deps/spdlog/include/**.h')
  add_files('deps/spdlog/src/*.cpp')
  add_deps('fmt')
  if is_plat('linux') then
    add_links('pthread')
  end

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
  end
  add_files('deps/nativefiledialog-extended/src/'..nfdimp)
  if is_plat('windows') or is_plat('msys') then
    add_links('ole32', 'uuid', 'gdi32', 'comctl32')
  end

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

target('miniz')
  set_kind('static')
  add_headerfiles('deps/miniz/miniz.h')
  add_files('deps/miniz/miniz.c')
  add_includedirs('deps/miniz', {public=true})

target('fmt')
  set_kind('static')
  add_headerfiles('deps/fmt/include/**.h')
  add_files('deps/fmt/src/format.cc', 'deps/fmt/src/os.cc')
  add_includedirs('deps/fmt/include', {public=true})

target('ngdoc')
  set_kind('static')
  add_headerfiles('include/nged/ngdoc.h')
  add_files('src/ngdoc.cpp', 'src/ngdraw.cpp', 'src/style.cpp')
  add_deps('spdlog', 'miniz', 'fmt')
  add_includedirs(
    'include',
    'deps/nlohmann',
    'deps/stduuid/include',
    'deps/stduuid', -- for gsl
    'deps/parallel_hashmap/parallel_hashmap',
    {public=true})

target('nged')
  set_kind('static')
  add_headerfiles('include/nged/*.h|ngdoc.h')
  add_files('src/nged.cpp', 'src/nged_imgui.cpp', 'src/nged_imgui_fonts.cpp')
  add_deps('spdlog', 'nfd', 'imgui', 'boxer', 'ngdoc', 'entry')
  add_cxflags('/bigobj', {tools='cl'})
  add_includedirs(
    'include',
    'deps/boxer/include',
    'deps/nlohmann',
    'deps/parallel_hashmap/parallel_hashmap'
  )

target('tests')
  set_kind('binary')
  add_deps('ngdoc', 'spdlog')
  add_files('tests/*.cpp')
  add_includedirs(
    '.',
    'deps/doctest')

target('lua')
  set_kind('static')
  add_cxflags('-fPIC', {tools = {'gcc', 'clang'}})
  add_includedirs('deps/lua')
  add_files('deps/lua/*.c|lua.c|luac.c|onelua.c')
  add_defines("LUA_COMPAT_5_3") -- Try adding compatibility flags if needed

target('entry')
  set_kind('static')
  add_deps('imgui')
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
  end
  if is_plat('windows') then
    add_links('ws2_32', 'advapi32', 'gdi32', 'shell32', 'version')
    if backend~='dx11' and backend~='dx12' then
      add_packages('vcpkg::glfw3')
    end
    if backend=='gl2' or backend=='gl3' then
      add_packages('vcpkg::gl3w')
    end
  else
    add_links('iconv')
  end

  if is_plat('macosx') then
    add_frameworks('AppKit', 'IOKit', 'QuartzCore')
    if backend=='metal' then
      add_frameworks('Metal', 'Foundation')
    end
  end

target('demo')
  set_kind('binary')
  add_deps('nged', 'entry')
  add_files('examples/demo/main.cpp')
  add_includedirs('.', 'deps/boxer/include')

target('typed_demo')
  set_kind('binary')
  add_deps('nged', 'entry')
  add_files('examples/typed_demo/main.cpp')
  add_includedirs('.', 'deps/boxer/include')

target('ngs7')
  set_kind('binary')
  add_deps('nged', 's7', 'entry')
  add_files('examples/ngs7/ngs7.cpp', 'examples/ngs7/main.cpp')
  add_includedirs('deps/imgui/backends')
  if is_plat('windows') then
    add_files('examples/ngs7/icon.rc')
  end
  add_includedirs('.', 'deps/boxer/include', 'deps/imgui', 'deps/nlohmann', 'deps/subprocess.h')

task('pytest')
  set_menu({
    usage = "xmake pytest",
    description = "Run python tests with the built extension",
    options = {}
  })
  on_run(function ()
    import("core.project.project")
    import("core.project.config")

    -- Load the project configuration (from .xmake/...)
    config.load()

    -- Ensure ngpy is built
    os.exec("xmake build ngpy")

    -- Load targets now that config is loaded
    project.load_targets()
    local target = project.target("ngpy")

    import("lib.detect.find_tool")
    local python = get_config("python")
    if not python or python == "auto" or python == "no" then
      local tool = find_tool("python3") or find_tool("python")
      if tool then python = tool.program end
    end
    if not python then python = "python3" end

    -- Now target:targetfile() should be correct
    local targetfile = target:targetfile()
    print("Built extension at: " .. targetfile)
    
    -- Copy to nged/ to ensure it's importable as 'nged.ngpy'
    local dest = path.join(os.projectdir(), "nged", path.filename(targetfile))
    print("Copying to " .. dest .. "...")
    os.cp(targetfile, dest)
    
    -- Setup environment
    local envs = {}
    envs.PYTHONPATH = os.projectdir()
    
    print("Running tests/pytest.py...")
    os.runv(python, {'tests/pytest.py'}, {envs = envs})
  end)

task('pydemo_test')
  set_menu({
    usage = "xmake pydemo_test",
    description = "Run headless tests for the pydemo example",
    options = {}
  })
  on_run(function ()
    import("core.project.project")
    import("core.project.config")

    config.load()
    os.exec("xmake build ngpy")

    project.load_targets()
    local target = project.target("ngpy")

    import("lib.detect.find_tool")
    local python = get_config("python")
    if not python or python == "auto" or python == "no" then
      local tool = find_tool("python3") or find_tool("python")
      if tool then python = tool.program end
    end
    if not python then python = "python3" end

    local targetfile = target:targetfile()
    local dest = path.join(os.projectdir(), "nged", path.filename(targetfile))
    os.cp(targetfile, dest)

    local envs = {}
    envs.PYTHONPATH = os.projectdir()

    print("Running examples/pydemo/test_headless.py...")
    os.execv(python, {'examples/pydemo/test_headless.py'}, {envs = envs})
  end)
