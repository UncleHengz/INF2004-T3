/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Define constants for timeout and error handling
#define SCAN_INTERVAL_MS 10000

// Function prototypes for error handling and resource management
void handle_scan_error(int error_code);
void cleanup();

static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result) {
        printf("SSID: %-32s, RSSI: %4d, Channel: %3d, MAC: %02x:%02x:%02x:%02x:%02x:%02x, Security: %u\n",
               result->ssid, result->rssi, result->channel,
               result->bssid[0], result->bssid[1], result->bssid[2],
               result->bssid[3], result->bssid[4], result->bssid[5], result->auth_mode);

        // Add additional checks or actions based on scan results if necessary

    } else {
        printf("Null result received in scan_result function.\n");
        // Perform error handling for null result if necessary
    }
    return 0;
}


int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43.\n");
        return 1;
    }

    // Enable station mode to allow for sniffing of networks
    cyw43_arch_enable_sta_mode();

    absolute_time_t scan_time = make_timeout_time_ms(SCAN_INTERVAL_MS);
    bool scan_in_progress = false;
    while (true) {
        if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
            if (!scan_in_progress) {
                cyw43_wifi_scan_options_t scan_options = {0};
                int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
                if (err == 0) {
                    printf("\nPerforming Wi-Fi scan\n");
                    scan_in_progress = true;
                } else {
                    handle_scan_error(err);
                    scan_time = make_timeout_time_ms(SCAN_INTERVAL_MS); // wait for the next scan
                }
            } else if (!cyw43_wifi_scan_active(&cyw43_state)) {
                scan_time = make_timeout_time_ms(SCAN_INTERVAL_MS); // wait for the next scan
                scan_in_progress = false;
            }
        }
    }

    cleanup();
    return 0;
}

void handle_scan_error(int error_code) {
    printf("Scan error occurred with code: %d\n", error_code);
    // Perform additional error handling if necessary
}

void cleanup() {
    cyw43_arch_deinit();
    // Perform any necessary cleanup operations here
}