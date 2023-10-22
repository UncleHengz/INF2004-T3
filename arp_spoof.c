#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ping.h"
#include "hardware/gpio.h"
#include "lwip/inet.h" 
#include "lwip/ip4_addr.h"
#include "lwip/etharp.h"

#ifndef PING_ADDR
#define PING_ADDR "192.168.19.240"
#endif

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

void main_task(__unused void *params) 
{
    //LWIP_PLATFORM_DIAG(("\nlwIP diagnostics enabled.\n"));

    if (cyw43_arch_init()) 
    {
        printf("** Failed to initialize **\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");

    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) 
    {
        // Failed to connect, retry after a delay
        printf("Failed to connect. Retrying in 30 seconds...\n");
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }

    printf("Connected.\n");

    // Get the device's IP address
    struct netif* netif = &cyw43_state.netif[0];
    ip_addr_t ip = netif->ip_addr;

    // Get the first three octets from the device's IP address
    uint8_t octet1 = ip4_addr1_16(&ip);
    uint8_t octet2 = ip4_addr2_16(&ip);
    uint8_t octet3 = ip4_addr3_16(&ip);

    printf("Device IP: %u.%u.%u.%u\n\n", octet1, octet2, octet3, ip4_addr4_16(&ip));

    // Define a range of IP addresses to scan within the same subnet
    ip4_addr_t start_ip, end_ip;
    IP4_ADDR(&start_ip, octet1, octet2, octet3, 1);
    IP4_ADDR(&end_ip, octet1, octet2, octet3, 254);


    while (true) 
    {
        for (uint32_t i = ip4_addr_get_u32(&start_ip); i <= ip4_addr_get_u32(&end_ip); i++) 
         {
            ip_addr_t target_ip;
            uint8_t octet4 = i & 0xFF;
            IP4_ADDR(ip_2_ip4(&target_ip), octet1, octet2, octet3, octet4);
            // Prepare the ARP request
            etharp_request(&cyw43_state.netif[0], &target_ip);

            // Delay a bit to allow time for a response (adjust as needed)
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        size_t entry_index;
        ip4_addr_t *ipaddr;
        struct netif *netif;
        struct eth_addr *ethaddr;

        // Iterate through ARP table entries
        for (entry_index = 0; entry_index < ARP_TABLE_SIZE; entry_index++) 
        {
            if (etharp_get_entry(entry_index, &ipaddr, &netif, &ethaddr)) 
            {
                // Print or use the information from the ARP table entry
                printf("Entry %d: IP Address: %s, Interface: %s, MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    entry_index, ip4addr_ntoa(ipaddr), netif->name, ethaddr->addr[0], ethaddr->addr[1],
                    ethaddr->addr[2], ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5]);
            }
        }
        vTaskDelay(1000);
    }
    cyw43_arch_deinit();
}

void vLaunch( void) 
{
    TaskHandle_t task;
    xTaskCreate(main_task, "TestMainThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &task);

#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
    // we must bind the main task to one core (well at least while the init is called)
    // (note we only do this in NO_SYS mode because cyw43_arch_freertos
    // takes care of it otherwise)
    vTaskCoreAffinitySet(task, 1);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main( void )
{
    stdio_init_all();

    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
#if ( portSUPPORT_SMP == 1 )
    rtos_name = "FreeRTOS SMP";
#else
    rtos_name = "FreeRTOS";
#endif

#if ( portSUPPORT_SMP == 1 ) && ( configNUM_CORES == 2 )
    printf("Starting %s on both cores:\n", rtos_name);
    vLaunch();
#elif ( RUN_FREERTOS_ON_CORE == 1 )
    printf("Starting %s on core 1:\n", rtos_name);
    multicore_launch_core1(vLaunch);
    while (true);
#else
    printf("Starting %s on core 0:\n", rtos_name);
    vLaunch();
#endif
    return 0;
}
