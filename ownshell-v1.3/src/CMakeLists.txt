add_subdirectory(core)
add_subdirectory(data_structure)

add_executable(ownshell main.c)

target_include_directories(ownshell PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(ownshell PRIVATE ownshell_core ownshell_data_structure)
