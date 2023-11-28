# WIFI Sniffing + Analysis

## Overview
### Original Objective
The team's original goal was to have 1 Raspberry Pi Pico W be used for ARP Spoofing, while another Raspberry Pi Pico W would be to capture, analyse, and save the network packets.
ARP Spoofing makes use of lwIP which is an open-source TCP/IP stack. The program will first try to connect to a specified Wifi network.
It will scan the network for the hosts on it.
After which, it will select a target and a gateway.
It will check if the char 'S' has been sent, if not, it will send an ARP packet to the target and the gateway. After which it will disconnect from the Wifi network.

In the process, a victim will be connected to the same WiFi network and surfing any website. If the victim surfs a vulnerable website, such as demo.testfire.net and keys certain credentials. The other Pico will capture those network packets.

The Network Capturing Pico uses Nexmon to capture the packets in promiscuous mode and is able to save them onto an SD card with the use of the FatFS_SPI while the Pico is connected with the Maker Pi Pico.

These network packets are obtained using a payload and then saved in its raw hexadecimal form in a txt file on the pico.

The Pico will then be able to open the txt file and convert the hexadecimal into clear text in order to analyse the packets to obtain the username and password the victim as inputted in the vulnerable website.

However, we were not able to transfer the network traffic from the ARP Spoofing Pico to the Network Capturing Pico and the IP forwarding when conducting the ARP spoofing is extremely laggy and slow even when using 1 Pico for conducting ARP Spoofing and 1 Pico to capture the network packets. Also, nexmon does not allow us to capture the network packets that we want. During testing, we were only able to obtain beacon packets which were redundant to us.

### Achieved Outcome
Instead, only 1 Raspberry Pi Pico is used. The pico will use the CYW43439 library to create a DHCP server and a web server on it. However, this does not provide internet access to hosts that connects to this DHCP server. The hosts that connect to this access point will only be able to access the web server. On the web server, there is a simple login page with 2 input text boxes: 'Username', 'Password'. Upon filling up both fields and the submit is pressed, the pico is able to obtain all traffic that passes through its network.

These traffic are payloads which are raw hexadecimal form and will be printed onto the pico's serial monitor. At the same time, these payloads will be saved onto an SD card which is connected to the Maker Pi Pico using the FatFS_SPI library. Then, we will be able to take the SD card & connect it to any PC and run the Python script to analyse the packets.

The Python script first opens the text file from the SD card and converts all hexadecimal into a readable format in UTF-8. The script will then filter keywords 'Username' & 'Password' along with the texts that were inputted in those fields and plot a graph to show the top 5 most commonly used usernames and passwords with the use of Python's matplotlib library.

## Libary Requirements
- lwIP
- FatFS_SPI
- Cyw43439

## Task Allocation
- ARP Spoofing: Teng Hui
- SD Card Read/Write: Steven
- Packet Monitoring/Capture: Teng Jun
- Packet Analysis: Jin Yuan & Yi Heng

## Future Changes/ Improvement
Potential improvement is to include packet analysis code in the pico. 
Integrate SD card and Nexmon into a combine code.
