cmake_minimum_required(VERSION 3.14)

add_library(${PROJECT_NAME}-arch OBJECT
    head.S
    main.c
    monitor.c
    common/tools.S
    common/uart.c
    common/printk.c
)
