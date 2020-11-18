# ChCore

This is the repository of ChCore labs in SE315, 2020 Spring.

本仓库为陈海波银杏书《现代操作系统：原理与实现》配套实验代码个人答案。不同实验的代码请通过切换 branch 查看，branch 名字为 lab<n>-howard，分别对应第 n 个实验的代码。

原始代码以及实验环境可以到 https://www.cnmooc.org/study/initplay/713843.mooc 获取。

实验参考手册可以到 https://ipads.se.sjtu.edu.cn/mospi/ 免费下载。 

如果发现我的代码有错误或者有更好的实现，欢迎到 Issues 留下你的看法，互相交流学习！

## build 
  - `make` or `make build`
  - The project will be built in `build` directory.

## Emulate
  - `make qemu`

  Emulate ChCore in QEMU

## Debug with GBD

  - `make qemu-gdb`

  Start a GDB server running ChCore
  
  - `make gdb`
  
  Start a GDB (gdb-multiarch) client

## Grade
  - `make grade`
  
  Show your grade of labs in the current branch

## Other
  - type `Ctrl+a x` to quit QEMU
  - type `Ctrl+d` to quit GDB
  
