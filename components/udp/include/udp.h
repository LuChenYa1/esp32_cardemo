#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// UDP 接收回调类型：data 为接收数据，len 为长度，addr 为发送方地址字符串，port 为发送方端口
typedef void (*udp_recv_cb_t)(const char *data, int len, const char *addr, uint16_t port);

// 启动 UDP 接收（绑定本地端口），返回 0 成功
int udp_start(uint16_t port);

// 发送到指定主机/端口（点分十进制主机地址）
int udp_send_to(const char *host, uint16_t port, const char *data, int len);

// 发送广播到指定端口
int udp_send_broadcast(uint16_t port, const char *data, int len);

// 设置接收回调，NULL 取消回调
void udp_set_recv_callback(udp_recv_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // UDP_H
