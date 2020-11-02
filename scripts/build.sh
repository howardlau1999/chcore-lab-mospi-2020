#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
    cd build
    cat > simulate.sh <<EOF
#!/bin/bash

basedir=\$(dirname "\$0")
if [[ \$@ == *"gdb"* ]]; then
	qemu-system-aarch64 -machine raspi3 -nographic -serial null -serial mon:stdio -m size=1G -kernel \$basedir/kernel.img -S -gdb tcp::1234
else
	qemu-system-aarch64 -machine raspi3 -nographic -serial null -serial mon:stdio -m size=1G -kernel \$basedir/kernel.img
fi
EOF
    chmod +x simulate.sh
    cd ..
fi

# compile kernel
echo "compiling kernel ..."
cd build

cmake -DCMAKE_LINKER=aarch64-linux-gnu-ld -DCMAKE_C_LINK_EXECUTABLE="<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" .. -G Ninja "$@"
echo "before ninja"
ninja
echo "after ninja"
if [ ! -f "kernel.img" ]; then
    echo "kernel.img is not exist"
fi
aarch64-linux-gnu-nm -n kernel.img > kernel.sym
# print the compile logs
#ninja -v
