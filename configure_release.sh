#!/bin/sh
rm -rf build && meson setup build --buildtype release
echo 'Configured. Run ninja -v -C build'
