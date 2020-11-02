cmake_minimum_required(VERSION 3.14)

file(
    GLOB
        init_source_files
        "init.c"
        "init_main.c"
)
add_executable(init.bin ${init_source_files})
target_link_libraries(init.bin chcore-user-lib)
set_property(
	TARGET init.bin
	APPEND_STRING
	PROPERTY
	LINK_FLAGS
	"-e START"
)

file(
    GLOB
        init_test_files
        "init.c"
        "init_test.c"
)
add_executable(init_test.bin ${init_test_files})
target_link_libraries(init_test.bin chcore-user-lib)
set_property(
	TARGET init_test.bin
	APPEND_STRING
	PROPERTY
	LINK_FLAGS
	"-e START"
)
