# WIFI Sniffing + Analysis
Our team has decided to program the Pico W to sniff for WIFI networks, followed by the hosts that are connected to the network.

After which, the Pico W will run ARP Spoofing which allows it to capture any username and password inputted by a victim if he/she
enters a website with no SSL to encrypt their password inputs. For this, we will be using demo.testfire.net to demonstrate this.

The Pico will be able to capture these network traffic like Wireshark and will then be saved onto a Micro SD card as a pcap file.

This pcap file can be opened by the "attacker" on another Pico to examine the network traffic and also see if the
victim has visited the malicious website and inputted his/her credentials.

ARP Spoofing is done using lwip framework. It will send out ARP request to all devices over the connected network.

For saving to the SD card, it makes use of the Micro SD Card Slot which then uses the FatFS_SPI driver on the pico.

Capturing of packets utilizes Nexmon which is a patch for the Pico.
