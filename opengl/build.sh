CC=clang CXX=clang++ meson setup build && ninja -v -C build

# repeated builds
ninja -v -C build

# after changing the build config:
meson setup build --wipe

