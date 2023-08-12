project('blurring', 'c', 'cpp', default_options : ['c_std=c17', 'cpp_std=c++20', 'warning_level=2', 'werror=true', 'b_sanitize=address,undefined'])


src = ['main.c', 'debug.cpp', 'util.c']
deps = [dependency('gl'), dependency('glew'), dependency('egl'), dependency('gbm')]
add_project_arguments('-D_POSIX_C_SOURCE=200112L', '-Wno-unused-parameter', '-Wno-unused-const-variable', '-Wno-unused-variable', language : ['c', 'cpp'])

executable(meson.project_name(), src, dependencies : deps )

# default is debug build, see --buildtype