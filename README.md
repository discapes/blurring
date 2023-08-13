# blurring

Simple program to blur an image, do edge detection or other convolution kernels. Uses the EGL DRM
platform to initialize OpenGL, so it can be run even without a display server. Run the program without 
arguments or with -h to see usage.

- Compiletime dependencies: meson
- Runtime dependencies: mesa, glew, glfw, stb_image

Building: 

- run ./configure_release.sh OR ./configure_debug.sh
- run ninja -C build

Some examples:

- `./build/blurring -i images/mountain-medium.jpg -o output.bmp`
- `time ./build/blurring -i images/mountain-medium.jpg -o output.bmp`
- `./build/blurring -i images/mountain-medium.jpg -o output.bmp -c /dev/dri/renderD129`
- `./build/blurring -i images/mountain-medium.jpg -g`
- `./build/blurring -i images/mountain-medium.jpg -o output.bmp -f edge.frag -c /dev/dri/renderD129`

Notes:

- When using an Nvidia render node in debug configuration, you might get
	LeakSanitizer warnings. To suppress them, either disable sanitization in
	configure_debug.sh, or `export LSAN_OPTIONS=fast_unwind_on_malloc=0,suppressions=lsan.supp`
- There's also a TypeScript program to do the same thing, inside `./nodejs-impl/`. It can be run
	with `pnpm -C nodejs-impl install && pnpm -C nodejs-impl/ start $PWD/images/mountain-medium.jpg $PWD/output.bmp 3 6`
	for around same amount of blur. It looks a bit nicer, but is ~70x slower.