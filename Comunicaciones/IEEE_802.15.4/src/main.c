#include <zephyr/net/net_pkt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <string.h>

LOG_MODULE_REGISTER(ieee_demo, LOG_LEVEL_DBG);

static struct net_if_link_cb link_cb;

static void recv_cb(struct net_if *iface, struct net_pkt *pkt)
{
    uint8_t data[128] = {0};
    int len = net_pkt_read(pkt, data, sizeof(data) - 1);

    if (len > 0) {
        data[len] = '\0';
        LOG_INF("📥 Received: %s");
    }

    net_pkt_unref(pkt);
}

static void send_packet(struct net_if *iface)
{
    const char *msg = "📡 Ping desde Zephyr!";
    size_t len = strlen(msg);

    struct net_pkt *pkt = net_pkt_alloc_with_buffer(iface, len, AF_UNSPEC, 0, K_NO_WAIT);
    if (!pkt) {
        LOG_ERR("❌ No se pudo alocar el paquete");
        return;
    }

    if (net_pkt_write(pkt, msg, len)) {
        LOG_ERR("❌ No se pudo escribir en el paquete");
        net_pkt_unref(pkt);
        return;
    }

    net_pkt_set_iface(pkt, iface);

    if (net_send_data(pkt) < 0) {
        LOG_ERR("❌ Error al enviar");
        net_pkt_unref(pkt);
    } else {
        LOG_INF("📤 Enviado: %s", msg);
    }
}

void main(void)
{
    struct net_if *iface = net_if_get_default();

    net_if_register_link_cb(&link_cb, recv_cb);

    if (!net_if_is_up(iface)) {
        net_if_up(iface);
        LOG_INF("✅ Interfaz activada");
    }

    while (1) {
        send_packet(iface);
        k_sleep(K_SECONDS(5));
    }
}
