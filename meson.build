project('blurring', 'c', 'cpp',	version : '1.0.0', 
	default_options : ['c_std=c17', 'cpp_std=c++20', 'warning_level=2', 'werror=true']
)

src = ['main.c', 'debug_output.cpp', 'util.c', 'debug.c']
deps = [dependency('gl'), dependency('glew'), dependency('egl'), dependency('gbm'), dependency('glfw3')]
add_project_arguments('-D_POSIX_C_SOURCE=200112L', '-Wno-unused-parameter',
    '-Wno-unused-const-variable', '-Wno-unused-variable', '-Wno-unused-function',
    language : ['c', 'cpp'])

executable(meson.project_name(), src, dependencies : deps )