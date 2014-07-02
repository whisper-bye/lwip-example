/*
 * mch_main.c
 *
 *  Created on: Jul 2, 2014
 *      Author: yonch
 */


#include "mch.h"
#include "lwip/inet.h"
#include "lwip/tcp.h"
#include "lwip/ip_frag.h"
#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "netif/etharp.h"

struct ip_addr mch_myip_addr;

#define MCH_ARP_TIMER_INTERVAL      (ARP_TMR_INTERVAL * 1000)
#define MCH_TCP_TIMER_INTERVAL      (TCP_TMR_INTERVAL * 1000)
#define MCH_IPREASS_TIMER_INTERVAL  (IP_TMR_INTERVAL * 1000)

static mch_timestamp ts_etharp;
static mch_timestamp ts_tcp;
static mch_timestamp ts_ipreass;

// Our network interface structure
static struct netif mchdrv_netif;

// Functions from my netif driver
// Probe function (find the device, return driver private data)
extern int mchdrv_probe(struct mch_pci_dev *, void **, uint8_t *);
// Init function
extern int mchdrv_attach(struct netif *);
// Poll for received frames
extern void mchdrv_poll(struct netif *);

int mch_net_init(void)
{
    struct ip_addr gw_addr, netmask;
    struct mch_pci_dev * mchdrv_pcidev;
    void * mchdrvnet_priv;
    uint8_t mac_addr[6];
    int err = -1;

    // Hard-coded IP for my address, gateway and netmask
    if (mch_net_aton(MCH_IPADDR_BASE, &mch_myip_addr))
        return -1;
    if (mch_net_aton(MCH_IPADDR_GW, &gw_addr))
        return -1;
    if (mch_net_aton(MCH_IPADDR_NETMASK, &netmask))
        return -1;

    // Initialize LWIP
    lwip_init();

    // Initialize PCI bus structure
    mch_pci_init();

    // Search through the list of PCI devices until we find our NIC
    mchdrv_pcidev = NULL;
    while ((mchdrv_pcidev = mch_pci_next(mchdrv_pcidev)) != NULL) {
        if ((err = mchdrv_probe(mchdrv_pcidev, &mchdrvnet_priv, mac_addr)) == 0)
            break;
    }

    if (mchdrv_pcidev == NULL) {
        mch_printf("mch_net_init: network adapter not found\n");
        return -1;
    }

    // Add our netif to LWIP (netif_add calls our driver initialization function)
    if (netif_add(&mchdrv_netif, &mch_myip_addr, &netmask, &gw_addr, mchdrvnet_priv,
                mchdrv_init, ethernet_input) == NULL) {
        mch_printf("mch_net_init: netif_add (mchdrv_init) failed\n");
        return -1;
    }

    netif_set_default(&mchdrv_netif);
    netif_set_up(&mchdrv_netif);

    // Initialize timer values
    mch_timestamp_get(&ts_etharp);
    mch_timestamp_get(&ts_tcp);
    mch_timestamp_get(&ts_ipreass);

    return 0;
}

//
// Regular polling mechanism.  This should be called each time through
// the main application loop (after each interrupt, regardless of source).
//
// It handles any received packets, permits NIC device driver house-keeping
// and invokes timer-based TCP/IP functions (TCP retransmissions, delayed
// acks, IP reassembly timeouts, ARP timeouts, etc.)
//
void mch_net_poll(void)
{
    mch_timestamp now;

    // Call network interface to process incoming packets and do housekeeping
    mchdrv_poll(&mchdrv_netif);

    // Process lwip network-related timers.
    mch_timestamp_get(&now);
    if (mch_timestamp_diff(&ts_etharp, &now) >= MCH_ARP_TIMER_INTERVAL) {
        etharp_tmr();
        ts_etharp = now;
    }
    if (mch_timestamp_diff(&ts_tcp, &now) >= MCH_TCP_TIMER_INTERVAL) {
        tcp_tmr();
        ts_tcp = now;
    }
    if (mch_timestamp_diff(&ts_ipreass, &now) >= MCH_IPREASS_TIMER_INTERVAL) {
        ip_reass_tmr();
        ts_ipreass = now;
    }
}

//
// Convert address from string to internal format.
// Return 0 on success; else non-zero
//
int mch_net_aton(char * str_addr, struct ip_addr * net_addr)
{
    struct in_addr a;
    int i = inet_aton(str_addr, &net_addr->addr);
    if (!i)
        return -1;
    return 0;
}

//
// Main entry point
//
int main(void)
{
    [snip other non-lwip initializations]
    mch_timestamp_init();       // Initialize timestamp generator
    mch_net_init();
    while (1) {
        [snip other non-lwip functions]
        mch_wait_for_interrupt();   // Awakened by network, timer or other interrupt
        mch_net_poll();             // Poll network stack
    }
}
