#!/bin/bash

echo "compiling user ..."

cd user

rm -rf build && mkdir build

C_FLAGS="-O3 -ffreestanding -Wall -fPIC -static"

echo "compiling: aarch64 user directory"
C_FLAGS="$C_FLAGS -DCONFIG_ARCH_AARCH64"

cd build
cmake .. -DCMAKE_C_FLAGS="$C_FLAGS" -G Ninja

ninja

echo "succeed in compiling user."

echo "building ramdisk ..."

mkdir -p ramdisk
echo "copy user/*.bin to ramdisk."
#cp lab3/*.bin ramdisk/
cp lab4/*.bin ramdisk/
cp lab5/apps/*.bin ramdisk/
echo "copy user/*.srv to ramdisk."
cp lab5/tmpfs/*.srv lab5/tmpfs/*.bin ramdisk/

cd ramdisk
echo "add fs_test files"
cpio -idmv < ../../lab5/tmpfs/fs_test.cpio 2> /dev/null
find . | cpio -o -Hnewc > ../ramdisk.cpio
echo "succeed in building ramdisk."
