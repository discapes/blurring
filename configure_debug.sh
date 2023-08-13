#!/bin/bash

# clang's faster for development
if command -v clang >/dev/null ; then
	export CC=clang
fi
if command -v clang++ >/dev/null ; then
	export CXX=clang++
fi

rm -rf build && meson setup build -Db_sanitize=address,undefined
echo 'Configured. Run ninja -v -C build'
