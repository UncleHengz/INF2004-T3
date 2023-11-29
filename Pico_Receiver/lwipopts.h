#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
// This example uses a common include to avoid repetition
#include "lwipopts_examples_common.h"

#if !NO_SYS
#define TCPIP_THREAD_STACKSIZE 1024
#define DEFAULT_THREAD_STACKSIZE 1024
#define DEFAULT_RAW_RECVMBOX_SIZE 8
#define TCPIP_MBOX_SIZE 8
#define LWIP_TIMEVAL_PRIVATE 0

// Enabling ARP, IPV4
#define LWIP_ARP 1
#define LWIP_IPV4 1

// Now going to raw packet manipulation
#define LWIP_PROVIDE_ERRNO 1
#define LWIP_RAW 1
#define LWIP_NETIF_API 1
#define LWIP_ETHERNET 1

#define IP_FORWARD 1
#define IP_FORWARD_ALLOW_TX_ON_RX_NETIF 1
#define IP_FRAG 1
#define IP_OPTIONS_ALLOWED 1
#define IP_REASSEMBLY 1

// Debug levels
//#define ETHARP_DEBUG LWIP_DBG_ON
#define IP_DEBUG LWIP_DBG_ON

// not necessary, can be done either way
#define LWIP_TCPIP_CORE_LOCKING_INPUT 1

// ping_thread sets socket receive timeout, so enable this feature
#define LWIP_SO_RCVTIMEO 1
#endif


#endif
