# This CmakeLists applies to both the Pico Spoofer and Receiver.

if (EXISTS ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c)
    add_executable(pico_freertos_arp_spoof_sys
            pico_freertos_arp_spoof.c
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
            )
    target_compile_definitions(pico_freertos_arp_spoof_sys PRIVATE
            WIFI_SSID="Peanut"
            WIFI_PASSWORD="Awesome123"
            NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
            LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
            PING_USE_SOCKETS=1
            )
    target_include_directories(pico_freertos_arp_spoof_sys PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping
            )
    target_link_libraries(pico_freertos_arp_spoof_sys
            pico_cyw43_arch_lwip_sys_freertos
            pico_stdlib
            pico_lwip_iperf
            FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
            )

        pico_enable_stdio_usb(pico_freertos_arp_spoof_sys 1)
        pico_enable_stdio_uart(pico_freertos_arp_spoof_sys 1)
        pico_add_extra_outputs(pico_freertos_arp_spoof_sys)
endif()
