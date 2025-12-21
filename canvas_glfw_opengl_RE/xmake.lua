-- 配置 spdlog 依赖（优先使用系统安装版本）
add_requires("spdlog", {system = true})
add_requires("cli11", {system = true})

add_rules("mode.debug", "mode.release")

-- compile toolchain
set_toolchains("gcc")
set_languages("c99", "c++17")
-- disable rtti
add_cxxflags("-fno-rtti", {tools = "gcc"})
-- for backtrace
add_cxxflags("-rdynamic",  {tools = "gcc"})

add_defines("INIT_WHITEBACKGROUND")
add_defines("PRINT_BACKTRACE")

if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
    set_strip("none")
end

-- base 库
target("base")
    set_kind("static")
    add_files("./include/base/*.cpp")
    add_includedirs("./include/")

-- skia filter 动态库
target("skia_filter")
    set_kind("static")
    add_files("./include/filters/*.cpp")
    add_includedirs("./include/")
    add_includedirs("/home/yanjiangha/mirrors/sda1_doc/temp_storage/", "/home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/")

-- ui 库
target("ui")
    set_kind("static")
    add_files("./include/ui/*.cpp")
    add_includedirs("./include/")
    add_includedirs("/home/yanjiangha/mirrors/sda1_doc/temp_storage/", "/home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/")

-- cache 库
target("cache")
    set_kind("static")
    add_files("./include/cache/*.cpp")
    add_includedirs("./include/")
    add_includedirs("/home/yanjiangha/mirrors/sda1_doc/temp_storage/", "/home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/")

target("compat")
    set_kind("static")
    add_files("./include/skia/compat/*.cpp")
    add_includedirs("./include/")
    add_includedirs("/home/yanjiangha/mirrors/sda1_doc/temp_storage/", "/home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/")

-- 定义目标类型为可执行程序
target("main")
    set_kind("binary")

    -- 添加源文件
    add_files("main_1.cpp")

    -- 配置头文件搜索路径（使用绝对路径更可靠）
    add_includedirs("/home/yanjiangha/mirrors/sda1_doc/temp_storage/", "/home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/")
    --add_includedirs("../common/")
    add_includedirs("./include/")

    -- 配置库文件搜索路径
    add_linkdirs(
        "/home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/", {force = true}
    )

    add_rpathdirs("/home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/")

    add_deps("ui", "skia_filter", "base", "cache", "compat")
    add_packages("spdlog")
    add_packages("cli11")

    -- 链接 skia 库
    add_links("skia", "skshaper", "skunicode_core", "skunicode_icu", "skparagraph", "skia_filter", "ui", "base", "cache","compat")
    -- glfw 引入
    add_links("glfw3", "glad", "GL")

    -- 生成 compile_commands.json（需要先安装插件）
    add_rules("plugin.compile_commands.autoupdate")

    -- xmake run 的执行环境目录，默认是二进制文件的目录
    set_rundir("$(projectdir)/")
