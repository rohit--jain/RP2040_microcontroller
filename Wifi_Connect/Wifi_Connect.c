#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mdns.h"
#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "pico/binary_info.h"

#define SSID "Network"
#define PASSWD "Morpheus#64285"

static void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                 const ip_addr_t *addr, u16_t port)
{
    printf("Received UDP packet from %s:%d, length %d\n",
           ipaddr_ntoa(addr), port, p->len);

    char buf[256] = {0};
    u16_t copy_len = (p->len < sizeof(buf) - 1) ? p->len : sizeof(buf) - 1;
    pbuf_copy_partial(p, buf, copy_len, 0);
    printf("Data: %s\n", buf);

    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    // sleep_ms(copy_len * 100);
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    pbuf_free(p);
}

int main()
{
    stdio_init_all();

    if (cyw43_arch_init())
    {
        printf("Wi-Fi init failed\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(SSID, PASSWD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("failed to connect.\n");
        return 1;
    }
    printf("Connected.\n");

    mdns_resp_init();
    mdns_resp_add_netif(netif_default, "pi");

    // Setup UDP receives
    struct udp_pcb *udp_pcb = udp_new();
    if (!udp_pcb)
    {
        printf("Failed to create UDP PCB\n");
        return 1;
    }
    if (udp_bind(udp_pcb, IP_ADDR_ANY, 5005) != ERR_OK)
    {
        printf("Failed to bind UDP PCB\n");
        return 1;
    }
    udp_recv(udp_pcb, udp_receive_callback, NULL);
    printf("UDP receiver ready on port 5005\n");

    while (true)
    {
        printf("Executing polling for receiving UDP messages");
        cyw43_arch_poll();
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        const char *ip_address_str = ip4addr_ntoa(netif_ip4_addr(netif_default));
        printf("IP address is %s\n", ip_address_str);
        sleep_ms(1000);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    }
}
