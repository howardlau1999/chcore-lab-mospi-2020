cmake_minimum_required(VERSION 3.11)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-unused-function")

file(
    GLOB
        tmpfs_source_files
        "tmpfs_main.c"
        "tmpfs_server.c"
        "tmpfs.c"
)

add_executable(tmpfs.srv
    ${tmpfs_source_files}
)

target_link_libraries(tmpfs.srv chcore-user-lib)

set_property(
	TARGET tmpfs.srv
	APPEND_STRING
	PROPERTY
	LINK_FLAGS
	"-e START"
)

file(
    GLOB
        tmpfs_test_files
        "tmpfs_test.c"
        "tmpfs_server.c"
        "tmpfs.c"
)

add_executable(fs_test.bin
    ${tmpfs_test_files}
)

target_link_libraries(fs_test.bin chcore-user-lib)

set_property(
	TARGET fs_test.bin
	APPEND_STRING
	PROPERTY
	LINK_FLAGS
	"-e START"
)
