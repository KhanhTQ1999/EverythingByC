add_library(ownshell_core STATIC
    shell.c
    shell_runtime.c
)

target_include_directories(ownshell_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(ownshell_core
    PUBLIC
        ownshell_data_structure
)
