# blurring

Simple program to blur an image. Uses the EGL DRM platform to initialize OpenGL, so 
it can be run even without a display server. Run the program without arguments or with
-h to see usage.

- Compiletime dependencies: meson
- Runtime dependencies: mesa, glew, glfw, stb_image

Building: 

- run ./configure_release.sh OR ./configure_debug.sh
- run ninja -C build

Example usage:

- `time ./build/blurring -i images/mountain-medium.jpg -o output.bmp -c /dev/dri/renderD129`
- `./build/blurring -i images/mountain-medium.jpg -g`

Notes:

- When using an Nvidia render node in debug configuration, you might get
	LeakSanitizer warnings. To suppress them, either disable sanitization in
	configure_debug.sh, or `export LSAN_OPTIONS=fast_unwind_on_malloc=0,suppressions=lsan.supp`
	