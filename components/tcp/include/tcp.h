#ifndef TCP_H
#define TCP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Start TCP slave (server) listening on given port. Returns 0 on success.
int tcp_slave_start(uint16_t port);

// Send data to currently connected client. Returns number of bytes sent, or -1 on error.
int tcp_slave_send(const char *data, int len);

// TCP 客户端：连接到远程主机
int tcp_client_connect(const char *host, uint16_t port);

// 连接到默认主机 192.168.130.125:3333
int tcp_client_connect_default(void);

// 向客户端/远程主机发送数据（如果作为客户端连接则发送到服务端）
int tcp_client_send(const char *data, int len);

// 主动断开客户端连接
void tcp_client_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif // TCP_H
