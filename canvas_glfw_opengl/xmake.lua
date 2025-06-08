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

-- 定义目标类型为可执行程序
target("main")
    set_kind("binary")

    -- 添加源文件
    add_files("main.cpp")

    -- 配置头文件搜索路径（使用绝对路径更可靠）
    add_includedirs("/home/yanjiangha/Android/", "/home/yanjiangha/Android/Skia")
    add_includedirs("../common/")

    -- 配置库文件搜索路径
    add_linkdirs(
        "/home/yanjiangha/Android/Skia/lib/Shared/", {force = true}
    )

    add_rpathdirs("/home/yanjiangha/Android/Skia/lib/Shared/")

    -- 链接 skia 库
    add_links("skia", "skshaper", "skunicode_core", "skunicode_icu", "skparagraph")
    -- glfw 引入
    add_links("glfw3", "glad", "GL")

    add_packages("spdlog")
    add_packages("cli11")

    -- 生成 compile_commands.json（需要先安装插件）
    add_rules("plugin.compile_commands.autoupdate")

    -- xmake run 的执行环境目录，默认是二进制文件的目录
    set_rundir("$(projectdir)/")
