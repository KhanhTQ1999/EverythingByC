add_library(ownshell_data_structure STATIC
    cstr.c
    vec.c
    ast.c
)

target_include_directories(ownshell_data_structure
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)
