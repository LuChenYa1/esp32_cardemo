/**
 * @file wifi.c
 * @brief WiFi连接和SNTP时间同步模块实现
 * 
 * 功能：
 * - WiFi STA模式连接
 * - SNTP时间同步
 * - 时间打印任务
 * 
 * 注意：
 * - WiFi与ADC2冲突，启用WiFi时ADC2不可用
 * - 需要先初始化NVS才能使用WiFi
 */

#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include <time.h>

/* 每 2 秒打印当前系统时间的任务（在 SNTP 同步后由 wifi 启动） */
static void time_print_task(void *pv)  // 定义一个静态函数，用于定时打印当前时间，参数pv未使用
{
	(void)pv;                           // 明确标记参数pv未被使用，避免编译警告
	for (;;) {                          // 无限循环执行以下代码
		time_t now;                     // 声明一个time_t类型的变量，用于存储当前时间戳
		struct tm timeinfo;             // 声明一个tm结构体变量，用于存储分解的时间信息
		time(&now);                     // 获取当前时间戳并存储到变量now中
		localtime_r(&now, &timeinfo);   // 将时间戳转换为本地时间结构体，存储到timeinfo中
		char buf[64];                   // 声明一个长度为64的字符数组，用于存储格式化后的时间字符串
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);  // 将timeinfo格式化为"年-月-日 时:分:秒"格式并存储到buf中
		ESP_LOGI("time", "当前时间: %s", buf);  // 以INFO级别输出当前时间，日志标签为"time"
		vTaskDelay(pdMS_TO_TICKS(2000));        // 延迟2000毫秒（2秒），然后继续循环
	}
}
static const char *TAG = "wifi_connect";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

/**
 * @brief WiFi事件处理函数
 * 
 * 该函数用于处理WiFi相关的事件，包括WiFi启动和断线重连等事件
 * 
 * @param arg 事件处理函数的参数指针，通常为NULL
 * @param event_base 事件基类型，标识事件来源的基础类型
 * @param event_id 具体的事件ID，用于区分不同的事件
 * @param event_data 事件数据指针，包含与事件相关的具体数据
 * @return 无返回值
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,  // 事件处理函数参数：用户自定义参数
                               int32_t event_id,                       // 事件ID
                               void* event_data)                       // 事件携带的数据
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {  // 检查是否为WiFi事件且是STA启动事件
        esp_wifi_connect();                                              // 启动WiFi连接
        ESP_LOGI(TAG, "wifi 开始, 连接中...");                           // 打印连接中的日志信息
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {  // 检查是否为WiFi断开连接事件
        // 处理WiFi断线事件，进行重连并清除连接状态位
        ESP_LOGW(TAG, "断开连接，正在重试...");                          // 打印重连警告信息
        esp_wifi_connect();                                              // 重新尝试连接WiFi
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);    // 清除WiFi连接成功标志位，表示未连接状态
    }
}

/**
 * @brief WiFi IP事件处理函数
 * 
 * 该函数用于处理WiFi站模式下的IP获取事件，当设备成功获取到IP地址时，
 * 会打印IP地址信息并设置WiFi连接成功的事件组位。
 * 
 * @param arg 事件处理函数的用户自定义参数（未使用）
 * @param event_base 事件基类型，用于标识事件来源
 * @param event_id 具体的事件ID
 * @param event_data 指向事件数据的指针，包含IP信息结构体
 * @return 无返回值
 */
static void ip_event_handler(void* arg, esp_event_base_t event_base,
							 int32_t event_id, void* event_data)
{
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		// 获取IP事件数据并转换为对应的结构体
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		// 打印获取到的IP地址信息
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		// 设置WiFi连接成功的事件组位
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

/**
 * @brief 初始化WiFi Station模式
 * 
 * 该函数负责初始化ESP32的WiFi Station模式，包括：
 * 1. NVS闪存初始化和错误处理
 * 2. 网络接口和事件循环初始化
 * 3. WiFi配置和启动
 * 4. 事件处理器注册
 * 5. 连接到指定的WiFi网络
 * 6. 时间同步(SNTP)配置和启动时间打印任务
 * 
 * @param void 无参数
 * @return void 无返回值
 */
void wifi_init_sta(void)
{ 
    // 初始化NVS（非易失性存储）以便存储WiFi配置等数据
    esp_err_t ret = nvs_flash_init();
    // 检查是否有页面不足或新版本发现错误，如有则擦除NVS后重新初始化
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化网络接口库和事件循环
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 为WiFi STA模式创建默认网络接口
    esp_netif_create_default_wifi_sta();

    // 使用默认配置初始化WiFi配置结构
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 创建事件组用于跟踪WiFi连接状态
    s_wifi_event_group = xEventGroupCreate();

    // 注册WiFi事件处理器，处理所有WiFi事件
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // 注册IP事件处理器，专门处理获取IP地址事件
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        NULL));

    // 配置WiFi连接参数
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,        // 目标WiFi网络名称
            .password = WIFI_PASS,    // WiFi密码
            .pmf_cfg = {              // 保护管理框架配置
                .capable = true,      // 设备支持PMF
                .required = false     // 不强制要求PMF
            }
        },
    };

    // 设置WiFi模式为STA（站点）模式，配置WiFi参数并启动WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta 完成.");

    // 等待WiFi连接完成，直到收到连接成功标志
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdTRUE,
                                           portMAX_DELAY);

    // 检查是否成功连接到WiFi
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "已连接到AP SSID:%s", WIFI_SSID);

        /* 初始化并启动 SNTP，同步系统时间 */
        ESP_LOGI(TAG, "开始初始化 SNTP");
        sntp_setoperatingmode(SNTP_OPMODE_POLL);  // 设置SNTP工作模式为轮询模式
        sntp_setservername(0, "pool.ntp.org");    // 设置NTP服务器地址
        sntp_init();                              // 启动SNTP服务

        /* 等待时间同步，最多等待 10 次（每次 2s） */
        time_t now = 0;
        struct tm timeinfo = { 0 };
        int retry = 0;
        const int retry_count = 10;
        time(&now);
        localtime_r(&now, &timeinfo);
        // 等待时间同步完成，检查年份是否大于2016年（判断时间是否已同步）
        while (timeinfo.tm_year < (2016 - 1900) && retry < retry_count) {
            ESP_LOGI(TAG, "等待时间同步... (%d/%d)", retry+1, retry_count);
            vTaskDelay(pdMS_TO_TICKS(2000));  // 延迟2秒
            time(&now);
            localtime_r(&now, &timeinfo);
            retry++;
        }
        // 如果时间同步成功
        if (timeinfo.tm_year >= (2016 - 1900)) {
            char strftime_buf[64];
            // 格式化时间字符串
            strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
            ESP_LOGI(TAG, "时间已同步: %s", strftime_buf);

            /* 启动时间打印任务，每2秒打印一次当前时间 */
            xTaskCreate(time_print_task, "time_print", 2048, NULL, tskIDLE_PRIORITY+1, NULL);
        } else {
            ESP_LOGW(TAG, "时间同步失败");  // 时间同步超时
        }
    } else {
        ESP_LOGE(TAG, "意外事件");  // 未收到预期的连接成功事件
    }
}