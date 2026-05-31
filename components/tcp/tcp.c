/**
 * @file tcp.c
 * @brief TCP通信模块实现
 * 
 * 功能：
 * - TCP服务器模式（监听客户端连接）
 * - TCP客户端模式（连接远程服务器）
 * - 数据收发接口
 * 
 * 注意：
 * - 使用互斥锁保护socket操作
 * - 需要先初始化WiFi才能使用TCP
 */

#include "tcp.h"               // 包含组件头文件
#include <string.h>            // 字符串操作头
#include <stdio.h>             // 标准输入输出头
#include <stdlib.h>            // 标准库（malloc/free）
#include <errno.h>             // errno 定义
#include "freertos/FreeRTOS.h" // FreeRTOS 主头
#include "freertos/task.h"     // 任务 API
#include "freertos/semphr.h"   // 信号量/互斥
#include "esp_log.h"           // ESP 日志 API
#include "lwip/sockets.h"      // LWIP sockets API

static const char *TAG = "tcp_slave";           // 日志 TAG
static int s_listen_sock = -1;                  // 监听 socket
static int s_client_sock = -1;                  // 当前客户端 socket
static SemaphoreHandle_t s_client_mutex = NULL; // 保护 s_client_sock 的互斥量
static uint16_t s_port = 0;                     // 监听端口
static int s_remote_sock = -1;                  /* 客户端模式使用的 socket */
static SemaphoreHandle_t s_remote_mutex = NULL; // 保护 s_remote_sock 的互斥量
static char s_remote_host[64] = {0};            // 保存重连主机地址
static uint16_t s_remote_port = 0;              // 保存重连端口
static TaskHandle_t s_reconnect_task = NULL;    // 后台重连任务句柄

/**
 * @brief TCP服务器主任务函数
 *
 * 该函数实现了一个TCP服务器，持续监听指定端口，接受客户端连接并进行数据回显。
 * 服务器会处理socket创建、绑定、监听等操作，并在出现错误时自动重试。
 *
 * @param pvParameters 任务参数指针（未使用）
 */
static void tcp_server_task(void *pvParameters) // 服务器主任务函数
{
    struct sockaddr_in server_addr;           // 服务器地址结构
    struct sockaddr_in client_addr;           // 客户端地址结构
    socklen_t addr_len = sizeof(client_addr); // 地址长度

    while (1)
    {                                                               // 持续监听循环
        int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP); // 创建 TCP socket
        if (listen_sock < 0)
        {                                                       // 创建失败处理
            ESP_LOGE(TAG, "创建 socket 失败: errno %d", errno); // 日志输出错误
            vTaskDelay(pdMS_TO_TICKS(2000));                    // 延迟后重试
            continue;                                           // 继续循环
        }

        int opt = 1;                                                          // 设置重用地址选项
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 设置 socket 选项

        server_addr.sin_family = AF_INET;                // IPv4
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意本地地址
        server_addr.sin_port = htons(s_port);            // 监听端口

        if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {                                                       // 绑定端口
            ESP_LOGE(TAG, "绑定 socket 失败: errno %d", errno); // 绑定失败日志
            close(listen_sock);                                 // 关闭 socket
            vTaskDelay(pdMS_TO_TICKS(2000));                    // 延迟后重试
            continue;                                           // 继续循环
        }

        if (listen(listen_sock, 1) < 0)
        {                                               // 开始监听
            ESP_LOGE(TAG, "监听失败: errno %d", errno); // 监听失败日志
            close(listen_sock);                         // 关闭 socket
            vTaskDelay(pdMS_TO_TICKS(2000));            // 延迟后重试
            continue;                                   // 继续循环
        }

        ESP_LOGI(TAG, "正在监听端口 %d", s_port); // 日志监听端口

        s_listen_sock = listen_sock; // 保存监听 socket

        // 接受客户端连接循环
        while (1)
        {                                                                                      // accept 循环
            int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len); // 接受连接
            if (client_sock < 0)
            {                                                   // accept 失败处理
                ESP_LOGE(TAG, "接受连接失败: errno %d", errno); // 日志错误
                break;                                          // 跳出以重启监听
            }

            char addr_str[INET_ADDRSTRLEN];                                              // 保存客户端地址字符串
            inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str));       // 转换地址
            ESP_LOGI(TAG, "客户端已连接: %s:%d", addr_str, ntohs(client_addr.sin_port)); // 日志客户端连接

            // store client socket
            if (s_client_mutex)
                xSemaphoreTake(s_client_mutex, portMAX_DELAY); // 获取互斥量
            s_client_sock = client_sock;                       // 保存客户端 socket
            if (s_client_mutex)
                xSemaphoreGive(s_client_mutex); // 释放互斥量

            // 与客户端通信循环 - 数据接收和回显
            while (1)
            {                                                                     // 与客户端通信循环
                char rx_buffer[512];                                              // 接收缓冲区
                int len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0); // 接收数据
                if (len < 0)
                {                                                   // 接收出错
                    ESP_LOGE(TAG, "接收数据失败: errno %d", errno); // 日志错误
                    break;                                          // 退出通信循环
                }
                else if (len == 0)
                {                                      // 客户端关闭连接
                    ESP_LOGI(TAG, "客户端已关闭连接"); // 日志关闭
                    break;                             // 退出通信循环
                }
                else
                {                                                        // 正常接收数据
                    rx_buffer[len] = '\0';                               // 添加字符串结束符
                    ESP_LOGI(TAG, "接收到 %d 字节: %s", len, rx_buffer); // 日志收到的数据
                    // Echo back the data
                    int to_write = len; // 需写回的数据量
                    int written = 0;    // 已写入计数
                    while (to_write > 0)
                    {                                                                // 循环发送直到全部写完
                        int w = send(client_sock, rx_buffer + written, to_write, 0); // 发送数据
                        if (w < 0)
                        {                                                     // 发送失败
                            ESP_LOGE(TAG, "发送数据时出错: errno %d", errno); // 日志发送错误
                            break;                                            // 退出发送循环
                        }
                        to_write -= w; // 减少剩余需要写入的字节
                        written += w;  // 增加已写入的字节数
                    }
                }
            }

            // close client
            if (s_client_mutex)
                xSemaphoreTake(s_client_mutex, portMAX_DELAY); // 获取互斥量
            if (s_client_sock == client_sock)
                s_client_sock = -1; // 清除保存的客户端 socket
            if (s_client_mutex)
                xSemaphoreGive(s_client_mutex); // 释放互斥量
            shutdown(client_sock, SHUT_RDWR);   // 关闭连接（双向）
            close(client_sock);                 // 关闭 socket
            ESP_LOGI(TAG, "客户端已断开");      // 日志断开
        }

        // close listener and retry after delay
        if (listen_sock != -1)
        {                                     // 清理监听 socket
            shutdown(listen_sock, SHUT_RDWR); // 关闭监听
            close(listen_sock);               // 关闭 socket
            s_listen_sock = -1;               // 重置监听句柄
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // 延迟后重新尝试监听
    }

    vTaskDelete(NULL); // 任务删除（理论上不可达）
}
/**
 * @brief TCP客户端接收任务
 *
 * 该函数作为FreeRTOS任务运行，负责接收来自远程服务器的数据，
 * 处理各种接收情况（成功、失败、连接关闭），并进行相应的资源清理。
 *
 * @param pvParameters 指向socket文件描述符的指针（传入后会被释放）
 * @return 无返回值
 */
static void tcp_client_recv_task(void *pvParameters)
{
    int sock = *((int *)pvParameters); // 获取传入的 socket
    free(pvParameters);                // 释放参数内存

    while (1)
    {
        char rx_buffer[512];                                       // 接收缓冲
        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0); // 接收数据

        // 处理接收结果：失败、连接关闭或正常接收
        if (len < 0)
        {                                                     // 接收失败
            ESP_LOGE(TAG, "客户端接收失败: errno %d", errno); // 日志错误
            break;                                            // 退出循环
        }
        else if (len == 0)
        {                                    // 远端关闭连接
            ESP_LOGI(TAG, "远端已关闭连接"); // 日志
            break;                           // 退出循环
        }
        else
        {                                                              // 正常接收
            rx_buffer[len] = '\0';                                     // 字符串终止
            ESP_LOGI(TAG, "客户端接收到 %d 字节: %s", len, rx_buffer); // 日志接收数据
        }
    }

    // 使用互斥量保护全局变量s_remote_sock的访问
    if (s_remote_mutex)
        xSemaphoreTake(s_remote_mutex, portMAX_DELAY); // 保护 s_remote_sock
    if (s_remote_sock == sock)
        s_remote_sock = -1; // 清理远端 socket
    if (s_remote_mutex)
        xSemaphoreGive(s_remote_mutex); // 释放互斥量

    // 关闭socket连接并删除当前任务
    shutdown(sock, SHUT_RDWR); // 关闭连接
    close(sock);               // 关闭 socket
    vTaskDelete(NULL);         // 删除任务
}

/* 内部连接函数：实际建立 socket、创建接收任务。返回 0 成功，-1 失败。 */
/**
 * @brief TCP客户端连接函数
 *
 * 该函数实现TCP客户端与指定主机和端口的连接功能，包括socket创建、
 * 连接建立、接收任务创建等操作，并将连接的socket保存到全局变量中。
 *
 * @param host 目标服务器的IP地址（点分十进制字符串格式）
 * @param port 目标服务器的端口号
 * @return int 成功返回0，失败返回-1
 */
int tcp_client_do_connect(const char *host, uint16_t port) // 内部连接实现
{
    struct sockaddr_in dest_addr;                // 远端地址结构
    dest_addr.sin_addr.s_addr = inet_addr(host); // 将点分十进制地址转为二进制
    dest_addr.sin_family = AF_INET;              // IPv4
    dest_addr.sin_port = htons(port);            // 设置远端端口

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP); // 创建客户端 socket
    if (sock < 0)
    {                                                             // 创建失败
        ESP_LOGE(TAG, "创建客户端 socket 失败: errno %d", errno); // 日志错误
        return -1;                                                // 返回错误
    }

    ESP_LOGI(TAG, "尝试连接到 %s:%d", host, port); // 日志尝试连接
    if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0)
    {                                               // 连接远端
        ESP_LOGE(TAG, "连接失败: errno %d", errno); // 连接失败日志
        close(sock);                                // 关闭 socket
        return -1;                                  // 返回错误
    }

    /* 保存远端 socket */
    if (s_remote_mutex)
        xSemaphoreTake(s_remote_mutex, portMAX_DELAY); // 获取远端互斥量
    s_remote_sock = sock;                              // 保存 socket
    if (s_remote_mutex)
        xSemaphoreGive(s_remote_mutex); // 释放互斥量

    /* 创建接收任务 */
    int *arg = malloc(sizeof(int)); // 为任务参数分配内存
    if (!arg)
    {                                  // 内存分配失败处理
        ESP_LOGE(TAG, "内存分配失败"); // 日志错误
        shutdown(sock, SHUT_RDWR);     // 关闭 socket
        close(sock);                   // 关闭 socket
        return -1;                     // 返回错误
    }
    *arg = sock; // 将 socket 放入参数
    if (xTaskCreate(tcp_client_recv_task, "tcp_client_recv", 4096, arg, tskIDLE_PRIORITY + 5, NULL) != pdPASS)
    {                                            // 创建接收任务
        ESP_LOGE(TAG, "创建客户端接收任务失败"); // 日志错误
        free(arg);                               // 释放内存
        shutdown(sock, SHUT_RDWR);               // 关闭 socket
        close(sock);                             // 关闭 socket
        return -1;                               // 返回错误
    }

    ESP_LOGI(TAG, "已连接到远端 %s:%d", host, port); // 连接成功日志
    return 0;                                        // 返回成功
}

/* 重连后台任务：如果断线，周期性尝试使用保存的 host/port 重连 */
/**
 * @brief TCP客户端后台重连任务
 *
 * 该任务负责监控TCP连接状态，当检测到连接断开且存在有效的远程主机配置时，
 * 会自动尝试重新连接。任务以无限循环方式运行，定期检查连接状态。
 *
 * @param pvParameters 任务参数（未使用，强制转换为void类型）
 */
static void tcp_client_reconnect_task(void *pvParameters) // 后台重连任务
{
    (void)pvParameters; // 未使用参数

    for (;;)
    { // 无限循环

        // 获取互斥量保护共享资源访问
        if (s_remote_mutex)
            xSemaphoreTake(s_remote_mutex, portMAX_DELAY); // 获取互斥量
        int sock = s_remote_sock;                          // 读取当前 socket
        // 释放互斥量
        if (s_remote_mutex)
            xSemaphoreGive(s_remote_mutex); // 释放互斥量

        // 检查是否需要进行重连操作
        if (sock < 0 && s_remote_host[0] != '\0' && s_remote_port != 0)
        {                                                                              // 未连接且有目标
            ESP_LOGI(TAG, "重连任务：尝试重连到 %s:%d", s_remote_host, s_remote_port); // 日志重连
            tcp_client_do_connect(s_remote_host, s_remote_port);                       // 尝试连接
            vTaskDelay(pdMS_TO_TICKS(5000));                                           // 失败后 5s 重试
        }
        else
        {                                     // 已连接或未配置目标
            vTaskDelay(pdMS_TO_TICKS(10000)); // 10s 后再检查
        }
    }
}

/**
 * @brief 主动断开TCP客户端连接
 *
 * 该函数用于安全地断开当前的TCP客户端连接。函数会先获取互斥锁来确保
 * 线程安全，然后获取当前socket句柄，将全局socket状态标记为未连接，
 * 最后释放互斥锁并关闭socket连接。
 *
 * @param void 无参数
 * @return void 无返回值
 */
void tcp_client_disconnect(void) // 主动断开客户端连接
{
    if (s_remote_mutex)
        xSemaphoreTake(s_remote_mutex, portMAX_DELAY); // 获取互斥
    int sock = s_remote_sock;                          // 获取当前 socket
    s_remote_sock = -1;                                // 将状态标记为未连接
    if (s_remote_mutex)
        xSemaphoreGive(s_remote_mutex); // 释放互斥

    // 检查是否存在有效的socket连接，如果存在则执行关闭操作
    if (sock >= 0)
    {                              // 如果有 socket，则关闭
        shutdown(sock, SHUT_RDWR); // shutdown
        close(sock);               // close
    }
}

/**
 * @brief TCP客户端连接函数
 *
 * 该函数用于建立TCP客户端连接，保存目标主机和端口信息，并尝试进行连接。
 * 如果连接失败，会启动后台重连任务来维持连接。
 *
 * @param host 目标服务器主机地址（字符串格式）
 * @param port 目标服务器端口号
 * @return int 连接结果，成功返回0，失败返回-1
 */
int tcp_client_connect(const char *host, uint16_t port) // 对外连接接口，保存 host/port 并尝试连接
{
    if (s_remote_mutex == NULL)
    {                                             // 首次调用时创建互斥
        s_remote_mutex = xSemaphoreCreateMutex(); // 创建互斥
        if (s_remote_mutex == NULL)
        {                                          // 失败处理
            ESP_LOGE(TAG, "创建客户端互斥锁失败"); // 日志错误
            return -1;                             // 返回失败
        }
    }

    /* 保存目标 host/port 以便重连任务使用 */
    strncpy(s_remote_host, host, sizeof(s_remote_host) - 1); // 保存主机地址
    s_remote_host[sizeof(s_remote_host) - 1] = '\0';         // 确保以 NUL 结尾
    s_remote_port = port;                                    // 保存端口

    /* 调用内部连接尝试函数 */
    int rc = -1; // 返回值
    rc = -1;     // 占位
    /* 先尝试连接一次 */
    extern int tcp_client_do_connect(const char *host, uint16_t port); // 声明内部函数
    rc = tcp_client_do_connect(host, port);                            // 尝试连接

    /* 如果重连任务未创建，启动一个后台任务负责断线重连 */
    if (s_reconnect_task == NULL)
    { // 检查是否已创建重连任务
        if (xTaskCreate((TaskFunction_t)tcp_client_reconnect_task, "tcp_client_reconn", 4096, NULL, tskIDLE_PRIORITY + 3, &s_reconnect_task) != pdPASS)
        {                                                            // 创建任务
            ESP_LOGW(TAG, "无法创建重连任务，客户端将不会自动重连"); // 日志警告
            s_reconnect_task = NULL;                                 // 保持为空
        }
    }

    return rc; // 返回结果
}

/**
 * @brief 连接到默认的TCP服务器地址和端口
 *
 * 该函数使用预设的IP地址(192.168.130.125)和端口号(3333)来建立TCP客户端连接
 *
 * @return int 返回连接结果，具体含义由tcp_client_connect函数定义
 *         - 成功时返回正值或0
 *         - 失败时返回负值或其他错误码
 */
int tcp_client_connect_default(void) // 连接默认地址和端口
{
    return tcp_client_connect("192.168.130.125", 3333); // 调用带参连接
}

/**
 * @brief TCP客户端发送数据函数
 *
 * 该函数用于向已连接的TCP服务器发送数据。函数会获取远端互斥量来安全地访问
 * 全局socket句柄，然后通过send系统调用将数据发送出去。支持部分发送，
 * 会循环发送直到所有数据都被发送完毕或发生错误。
 *
 * @param data 要发送的数据缓冲区指针
 * @param len 要发送的数据长度（字节数）
 * @return 成功时返回实际发送的字节数，失败时返回-1
 */
int tcp_client_send(const char *data, int len) // 客户端发送函数
{
    int ret = -1; // 返回值初始化
    if (s_remote_mutex)
        xSemaphoreTake(s_remote_mutex, portMAX_DELAY); // 获取远端互斥量
    int client = s_remote_sock;                        // 读取远端 socket
    if (s_remote_mutex)
        xSemaphoreGive(s_remote_mutex); // 释放互斥量

    if (client < 0)
    {                                        // 未连接
        ESP_LOGW(TAG, "客户端未连接到远端"); // 日志警告
        return -1;                           // 返回错误
    }

    // 计算需要发送的数据量并进行循环发送
    int to_write = len; // 剩余需要写入字节数
    int written = 0;    // 已写入字节数
    while (to_write > 0)
    {                                                      // 循环写入
        int w = send(client, data + written, to_write, 0); // 发送数据
        if (w < 0)
        {                                                     // 发送失败
            ESP_LOGE(TAG, "客户端发送失败: errno %d", errno); // 日志错误
            ret = -1;                                         // 标记失败
            break;                                            // 退出循环
        }
        to_write -= w; // 减少剩余
        written += w;  // 增加已写入
        ret = written; // 记录已写入
    }

    return ret; // 返回已写入字节数或 -1
}

/**
 * @brief 启动TCP主机服务器
 *
 * 该函数用于启动一个TCP服务器，监听指定端口并处理客户端连接
 *
 * @param port 要监听的端口号
 * @return int 成功返回0，失败返回-1
 */
int tcp_slave_start(uint16_t port) // 启动 server
{
    // 初始化互斥量
    if (s_client_mutex == NULL)
    {
        // 创建互斥
        s_client_mutex = xSemaphoreCreateMutex();
        // 失败处理
        if (s_client_mutex == NULL)
        {
            // 日志错误
            ESP_LOGE(TAG, "创建互斥锁失败");
            // 返回错误
            return -1;
        }
    }

    // 设置监听端口
    s_port = port;
    // 创建 server 任务
    BaseType_t r = xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, tskIDLE_PRIORITY + 5, NULL);
    // 失败处理
    if (r != pdPASS)
    {
        // 日志错误
        ESP_LOGE(TAG, "创建 tcp_server 任务失败");
        // 返回错误
        return -1;
    }
    // 成功返回
    return 0;
}

/**
 * @brief TCP从机发送函数
 *
 * 通过TCP连接向主机发送数据
 *
 * @param data 要发送的数据缓冲区指针
 * @param len 要发送的数据长度
 * @return int 成功时返回实际发送的字节数，失败时返回-1
 */
int tcp_slave_send(const char *data, int len) // 从机发送函数
{
    int ret = -1; // 返回值
    if (s_client_mutex)
        xSemaphoreTake(s_client_mutex, portMAX_DELAY); // 获取互斥
    int client = s_client_sock;                        // 读取当前客户端
    if (s_client_mutex)
        xSemaphoreGive(s_client_mutex); // 释放互斥

    if (client < 0)
    {                                    // 无客户端连接
        ESP_LOGW(TAG, "没有客户端连接"); // 日志警告
        return -1;                       // 返回错误
    }

    // 循环发送数据直到全部发送完成或发生错误
    int to_write = len; // 剩余写入
    int written = 0;    // 已写入
    while (to_write > 0)
    {                                                      // 循环发送
        int w = send(client, data + written, to_write, 0); // 发送数据
        if (w < 0)
        {                                               // 发送失败
            ESP_LOGE(TAG, "发送失败: errno %d", errno); // 日志错误
            ret = -1;                                   // 标记失败
            break;                                      // 退出循环
        }
        to_write -= w; // 减少剩余
        written += w;  // 累加已写入
        ret = written; // 更新返回值
    }

    return ret; // 返回已发送字节数或 -1
}
