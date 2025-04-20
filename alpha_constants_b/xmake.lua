-- 配置 spdlog 依赖（优先使用系统安装版本）
add_requires("spdlog", {system = true})

-- compile toolchain
set_toolchains("gcc")
set_languages("c99", "c++17")

-- 定义目标类型为可执行程序
target("alpha_contrants_b")
    set_kind("binary")

    -- 添加源文件
    add_files("main.cpp")

    -- 配置头文件搜索路径（使用绝对路径更可靠）
    add_includedirs("/home/yanjiangha/Android/", "/home/yanjiangha/Android/Skia")

    -- 配置库文件搜索路径
    add_linkdirs(
        "/home/yanjiangha/Android/Skia/lib/Shared/", {force = true}
    )

    add_rpathdirs("/home/yanjiangha/Android/Skia/lib/Shared/")

    -- 链接 skia 库
    add_links("skia", "skshaper", "skunicode_core", "skunicode_icu")

    add_packages("spdlog")

    -- 生成 compile_commands.json（需要先安装插件）
    add_rules("plugin.compile_commands.autoupdate")

    -- xmake run 的执行环境目录，默认是二进制文件的目录
    set_rundir("$(projectdir)/")
