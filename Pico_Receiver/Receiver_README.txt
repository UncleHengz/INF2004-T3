To use the receiver/monitor, please overwrite the default ip.c  and ip.h with these ones, along with using this specific lwipopts.h with the C file.
Use the provided CmakeLists.txt for both the receiver and spoofer, switching the SSID and password to the target WIFI network.

This receiver uses the various debug options, along with a modified IP.c and IP.h from the lwip library to print out the contents of IP packets received by the pico, before forwarding them to their destination since it's not meant for the Pico. The C code is there to ensure that the pico connects to the network and stays on.
