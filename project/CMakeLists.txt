add_executable(project project.c)

target_include_directories(project PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

target_link_libraries(project
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )

pico_enable_stdio_usb(project 1)

pico_add_extra_outputs(project)