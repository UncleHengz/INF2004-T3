// TH
#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ping.h"
#include "hardware/gpio.h"
#include "lwip/inet.h" 
#include "lwip/ip4_addr.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "lwip/raw.h"
#include "lwip/ip.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

// Define the Ethernet ethertype for ARP.
#define ARP_PROTOCOL 0x0806

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

// Function to connect to the target WIFI network.
void connectWIFI()
{
    if (cyw43_arch_init()) 
    {
        printf("** Failed to initialize **\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");

    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 15000)) 
    {
        // Failed to connect, retry after a delay.
        printf("Failed to connect. Retrying in 15 seconds...\n");
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
    printf("Connected.\n");
}

// Main task.
void main_task(__unused void *params) 
{
    connectWIFI();

    // Get the device's IP address.
    struct netif* netif = &cyw43_state.netif[0];
    ip_addr_t ip = netif->ip_addr;

    // Set device_mac using the MAC address of the netif.
    struct eth_addr device_mac;
    SMEMCPY(&device_mac, netif->hwaddr, ETH_HWADDR_LEN);

    // Get the first three octets from the device's IP address.
    uint8_t octet1 = ip4_addr1_16(&ip);
    uint8_t octet2 = ip4_addr2_16(&ip);
    uint8_t octet3 = ip4_addr3_16(&ip);

    // Prints out the device's information.
    printf("\nDevice IP: %u.%u.%u.%u\n", octet1, octet2, octet3, ip4_addr4_16(&ip));
    printf("Device MAC: %02X:%02X:%02X:%02X:%02X:%02X\n\n", device_mac.addr[0], device_mac.addr[1], 
    device_mac.addr[2], device_mac.addr[3], device_mac.addr[4], device_mac.addr[5]);    

    // While loop to have the main task run indefinitely.
    while(getchar_timeout_us(0) != 'S')
    {
        tight_loop_contents();
    }
    cyw43_arch_deinit();
}

void vLaunch( void) {
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
