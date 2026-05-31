/**
 * @file udp.c
 * @brief UDP通信模块实现
 * 
 * 功能：
 * - UDP数据收发
 * - 接收回调机制
 * - 后台接收任务
 * 
 * 注意：
 * - UDP是无连接协议，不保证数据可靠传输
 * - 需要先初始化WiFi才能使用UDP
 */

#include "udp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/sockets.h"

/* 日志标签，用于标识此模块的日志输出 */
static const char *TAG = "udp";

/* 全局变量定义 */
static int s_udp_sock = -1;            // 绑定用于接收的socket句柄，初始化为-1表示未创建
static TaskHandle_t s_udp_task = NULL; // UDP接收任务句柄，用于管理接收线程
static udp_recv_cb_t s_recv_cb = NULL; // 用户设置的接收回调函数指针

/**
 * @brief UDP数据接收任务
 *
 * 这是一个FreeRTOS任务，持续监听UDP socket上的数据，
 * 接收到数据后会解析发送方信息并调用用户注册的回调函数
 *
 * @param pvParameters 任务参数（未使用）
 */
static void udp_recv_task(void *pvParameters)
{
    (void)pvParameters; // 防止编译器警告未使用的参数

    struct sockaddr_in source_addr;          // 存储发送方的IP地址和端口信息
    socklen_t socklen = sizeof(source_addr); // 地址结构体大小
    char rx_buffer[512];                     // 接收数据缓冲区，最大可存储512字节

    // 持续接收数据循环
    while (1)
    {
        // 从UDP socket接收数据
        int len = recvfrom(s_udp_sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&source_addr, &socklen);

        // 检查接收是否成功
        if (len < 0)
        {
            ESP_LOGE(TAG, "接收 UDP 数据失败: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(1000)); // 延迟1秒后继续尝试
            continue;
        }

        // 在接收的数据末尾添加字符串结束符
        rx_buffer[len] = '\0';

        // 将网络地址转换为字符串格式
        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &source_addr.sin_addr, addr_str, sizeof(addr_str));

        // 获取发送方端口并转换为主机字节序
        uint16_t src_port = ntohs(source_addr.sin_port);

        // 记录接收到的数据信息
        ESP_LOGI(TAG, "接收到 UDP %s:%d -> %d 字节: %s", addr_str, src_port, len, rx_buffer);

        // 如果用户注册了回调函数，则执行回调
        if (s_recv_cb)
        {
            s_recv_cb(rx_buffer, len, addr_str, src_port);
        }
    }

    // 任务删除（实际上不会到达这里，因为上面是无限循环）
    vTaskDelete(NULL);
}

/**
 * @brief 启动UDP服务，绑定指定端口并开始接收数据
 *
 * 初始化UDP socket，绑定到指定端口，并创建接收任务
 *
 * @param port 要绑定的端口号
 * @return 成功返回0，失败返回-1
 */
int udp_start(uint16_t port)
{
    // 检查UDP是否已经启动
    if (s_udp_sock != -1)
    {
        ESP_LOGW(TAG, "UDP 已经启动");
        return 0;
    }

    // 创建UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "创建 UDP socket 失败: errno %d", errno);
        return -1;
    }

    // 配置监听地址信息
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;                // IPv4协议族
    listen_addr.sin_port = htons(port);              // 转换端口号为网络字节序
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有可用IP地址

    // 绑定socket到指定端口
    if (bind(sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0)
    {
        ESP_LOGE(TAG, "绑定 UDP 端口失败: errno %d", errno);
        close(sock); // 关闭socket
        return -1;
    }

    // 保存socket句柄
    s_udp_sock = sock;

    // 创建UDP接收任务
    if (xTaskCreate(udp_recv_task, "udp_recv", 4096, NULL, tskIDLE_PRIORITY + 5, &s_udp_task) != pdPASS)
    {
        ESP_LOGE(TAG, "创建 UDP 接收任务失败");
        close(sock);     // 关闭socket
        s_udp_sock = -1; // 重置全局socket句柄
        return -1;
    }

    ESP_LOGI(TAG, "UDP 已绑定端口 %d", port);
    return 0;
}

/**
 * @brief 向指定主机和端口发送UDP数据
 *
 * 创建临时UDP socket向目标地址发送数据，然后关闭socket
 *
 * @param host 目标主机IP地址（字符串格式）
 * @param port 目标端口号
 * @param data 要发送的数据缓冲区
 * @param len 要发送的数据长度
 * @return 成功时返回实际发送的字节数，失败时返回-1
 */
int udp_send_to(const char *host, uint16_t port, const char *data, int len)
{
    // 创建UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "创建发送 socket 失败: errno %d", errno);
        return -1;
    }

    // 设置目标地址信息
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;              // IPv4协议族
    dest_addr.sin_port = htons(port);            // 转换端口号为网络字节序
    dest_addr.sin_addr.s_addr = inet_addr(host); // 将IP字符串转换为网络地址

    // 发送UDP数据到目标地址
    int ret = sendto(sock, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret < 0)
    {
        ESP_LOGE(TAG, "udp sendto 失败: errno %d", errno);
    }

    // 关闭socket并返回发送结果
    close(sock);
    return ret;
}

/**
 * @brief 发送UDP广播数据
 *
 * 创建UDP socket并启用广播功能，向指定端口发送广播数据
 *
 * @param port 广播的目标端口号
 * @param data 要发送的数据缓冲区
 * @param len 要发送的数据长度
 * @return 成功时返回实际发送的字节数，失败时返回-1
 */
int udp_send_broadcast(uint16_t port, const char *data, int len)
{
    // 创建UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "创建广播 socket 失败: errno %d", errno);
        return -1;
    }

    // 启用socket的广播功能
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    // 设置广播地址信息
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;                      // IPv4协议族
    dest_addr.sin_port = htons(port);                    // 转换端口号为网络字节序
    dest_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); // 使用广播地址

    // 发送广播数据
    int ret = sendto(sock, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret < 0)
    {
        ESP_LOGE(TAG, "udp broadcast 发送失败: errno %d", errno);
    }

    // 关闭socket并返回发送结果
    close(sock);
    return ret;
}

/**
 * @brief 设置UDP数据接收回调函数
 *
 * 当接收到UDP数据时，会调用这个回调函数进行处理
 *
 * @param cb 用户定义的回调函数指针，函数原型为udp_recv_cb_t
 */
void udp_set_recv_callback(udp_recv_cb_t cb)
{
    s_recv_cb = cb; // 保存回调函数指针
}