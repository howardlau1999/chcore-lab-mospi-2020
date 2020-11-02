cmake_minimum_required(VERSION 3.14)

add_library(${PROJECT_NAME}-arch process
    head.S
    main.c
    monitor.c
    common/tools.S
    common/cpio.c
    common/elf.c
    common/uart.c
    common/printk.c
    common/radix.c
)
