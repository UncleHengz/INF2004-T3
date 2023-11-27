# WIFI Sniffing + Analysis

## Overview
The team's original goal was to have 1 Raspberry Pi Pico W be used for ARP Spoofing, while another Raspberry Pi Pico W would be to capture, analyse, and save the network packets.
ARP Spoofing makes use of lwIP which is an open-source TCP/IP stack. The program will first try to connect to a specified Wifi network.
It will scan the network for the hosts on it.
After which, it will select a target and a gateway.
It will check if the char 'S' has been sent, if not, it will send an ARP packet to the target and the gateway. After which it will disconnect from the Wifi network.

In the process, a victim will be connected to the same WiFi network and surfing any website. If the victim surfs a vulnerable website, such as demo.testfire.net and keys certain credentials. The other Pico will capture those network packets.

The Network Capturing Pico uses Nexmon to capture the packets in promiscuous mode and is able to save them onto an SD card with the use of the FatFS_SPI while the Pico is connected with the Maker Pi Pico.

These network packets are obtained using a payload and then saved in its raw hexadecimal form in a txt file on the pico.

The Pico will then be able to open the txt file and convert the hexadecimals into clear text in order to analyse the packets to obtain the username and password the victim as inputted in the vulnerable website.

##Requirements
- lwIP
- Nexmon
- Fat32_SPI
- idk what else

## Future Changes/ Improvement
Potential improvement is to include packet analysis code in the pico. 
Integrate SD card and Nexmon into a combine code.
