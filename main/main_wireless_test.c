// /**
//  * @file main_wireless_test.c
//  * @brief ESP32无线通信模块测试程序
//  * 
//  * 功能：测试WiFi、TCP、蓝牙BLE、SPP透传等功能
//  */

// #include "wireless.h"
// #include "esp_log.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include <string.h>

// static const char *TAG = "WIRELESS_TEST";

// // ========== 配置区域 ==========

// // WiFi配置
// #define WIFI_SSID "YourSSID"
// #define WIFI_PASSWORD "YourPassword"

// // TCP服务器配置
// #define TCP_SERVER_IP "192.168.1.100"
// #define TCP_SERVER_PORT 5001

// // BLE SPP配置（选择设备角色）
// #define BLE_DEVICE_ROLE_SERVER  1  // 1=服务端, 0=客户端

// // 服务端配置
// #define BLE_SERVER_NAME "ESP32_Server"
// #define BLE_SERVER_ADV_DATA "0201060A09457370726573736966030302A0"

// // 客户端配置（填写服务端MAC地址）
// #define BLE_SERVER_MAC "24:0a:c4:d6:e4:46"

// // SPP参数
// #define SPP_SERVER_TX_SRV   1
// #define SPP_SERVER_TX_CHAR  1
// #define SPP_SERVER_RX_SRV   7
// #define SPP_SERVER_RX_CHAR  1
// #define SPP_CLIENT_TX_SRV   1
// #define SPP_CLIENT_TX_CHAR  3
// #define SPP_CLIENT_RX_SRV   5
// #define SPP_CLIENT_RX_CHAR  3
// #define SPP_MTU_SIZE        5

// // ========== 测试函数 ==========

// /**
//  * @brief 测试1：基础AT指令
//  */
// void test_at_basic(void)
// {
//     ESP_LOGI(TAG, "========== 测试1：基础AT指令 ==========");
    
//     ESP_LOGI(TAG, "1. 测试AT指令...");
//     if (wireless_test()) {
//         ESP_LOGI(TAG, "   [OK] 模块响应正常");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL] 模块无响应");
//     }
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "2. 查询固件版本...");
//     wireless_send_at_command("AT+GMR");
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     ESP_LOGI(TAG, "3. 查询IP地址...");
//     wireless_query_ip();
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "测试完成\n");
// }

// /**
//  * @brief 测试2：WiFi连接
//  */
// void test_wifi_connect(void)
// {
//     ESP_LOGI(TAG, "========== 测试2：WiFi连接 ==========");
    
//     ESP_LOGI(TAG, "1. 设置Station模式...");
//     wireless_set_wifi_mode(1);
//     vTaskDelay(pdMS_TO_TICKS(500));
//     ESP_LOGI(TAG, "   (如果已经是Station模式，返回ERROR是正常的)");
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "2. 连接WiFi: %s...", WIFI_SSID);
//     ESP_LOGI(TAG, "   (等待10-20秒)");
//     if (wireless_connect_wifi(WIFI_SSID, WIFI_PASSWORD)) {
//         ESP_LOGI(TAG, "   [OK] WiFi连接成功");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL] WiFi连接失败");
//     }
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "3. 查询IP地址...");
//     wireless_query_ip();
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     ESP_LOGI(TAG, "4. Ping测试: baidu.com...");
//     if (wireless_ping("baidu.com")) {
//         ESP_LOGI(TAG, "   [OK]");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL]");
//     }
    
//     ESP_LOGI(TAG, "测试完成\n");
// }

// /**
//  * @brief 测试3：TCP客户端
//  */
// void test_tcp_client(void)
// {
//     ESP_LOGI(TAG, "========== 测试3：TCP客户端 ==========");
    
//     ESP_LOGI(TAG, "1. 设置单连接模式...");
//     wireless_set_single_connection();
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "2. 连接TCP服务器: %s:%d...", TCP_SERVER_IP, TCP_SERVER_PORT);
//     if (wireless_connect_tcp(TCP_SERVER_IP, TCP_SERVER_PORT)) {
//         ESP_LOGI(TAG, "   [OK]");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL]");
//         return;
//     }
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "3. 发送测试数据...");
//     const char *test_data = "Hello from ESP32!";
//     if (wireless_send_data((uint8_t *)test_data, strlen(test_data))) {
//         ESP_LOGI(TAG, "   [OK]");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL]");
//     }
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "4. 等待接收数据...");
//     uint8_t rx_buf[128];
//     vTaskDelay(pdMS_TO_TICKS(2000));
//     uint16_t len = wireless_get_received_data(rx_buf, sizeof(rx_buf));
//     if (len > 0) {
//         rx_buf[len] = '\0';
//         ESP_LOGI(TAG, "   接收到 %d 字节: %s", len, rx_buf);
//     } else {
//         ESP_LOGI(TAG, "   未接收到数据");
//     }
    
//     ESP_LOGI(TAG, "5. 断开连接...");
//     wireless_disconnect_tcp();
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "测试完成\n");
// }

// /**
//  * @brief 测试4：TCP透传
//  */
// void test_tcp_transparent(void)
// {
//     ESP_LOGI(TAG, "========== 测试4：TCP透传 ==========");
    
//     ESP_LOGI(TAG, "1. 设置单连接模式...");
//     wireless_set_single_connection();
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "2. 连接TCP服务器...");
//     if (!wireless_connect_tcp(TCP_SERVER_IP, TCP_SERVER_PORT)) {
//         ESP_LOGE(TAG, "   [FAIL]");
//         return;
//     }
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "3. 设置透传模式...");
//     wireless_set_transparent_mode(1);
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "4. 进入透传...");
//     if (wireless_enter_transparent()) {
//         ESP_LOGI(TAG, "   [OK] 已进入透传模式");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL]");
//         return;
//     }
    
//     ESP_LOGI(TAG, "5. 发送测试数据...");
//     for (int i = 0; i < 10; i++) {
//         char buf[32];
//         snprintf(buf, sizeof(buf), "Transparent #%d\r\n", i);
//         wireless_send_string(buf);
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     ESP_LOGI(TAG, "6. 退出透传...");
//     wireless_exit_transparent();
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "7. 恢复普通模式...");
//     wireless_set_transparent_mode(0);
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "8. 断开连接...");
//     wireless_disconnect_tcp();
    
//     ESP_LOGI(TAG, "测试完成\n");
// }

// /**
//  * @brief 测试5：蓝牙扫描
//  */
// void test_ble_scan(void)
// {
//     ESP_LOGI(TAG, "========== 测试5：蓝牙扫描 ==========");
    
//     ESP_LOGI(TAG, "1. 初始化蓝牙(Client模式)...");
//     if (wireless_ble_init(1)) {
//         ESP_LOGI(TAG, "   [OK]");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL]");
//         return;
//     }
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "2. 查询蓝牙地址...");
//     wireless_send_at_command("AT+BLEADDR?");
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "3. 扫描蓝牙设备(5秒)...");
//     if (wireless_ble_scan(1, 5)) {
//         ESP_LOGI(TAG, "   [OK]");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL]");
//     }
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "4. 关闭蓝牙...");
//     wireless_ble_init(0);
    
//     ESP_LOGI(TAG, "测试完成\n");
// }

// /**
//  * @brief 测试6：BLE SPP服务端
//  */
// void test_ble_spp_server(void)
// {
//     ESP_LOGI(TAG, "========== 测试6：BLE SPP服务端 ==========");
    
//     // 初始化
//     ESP_LOGI(TAG, "1. 初始化蓝牙(Server模式)...");
//     if (!wireless_ble_init(2)) { 
//         ESP_LOGE(TAG, "   [FAIL]"); 
//         return; 
//     }
//     ESP_LOGI(TAG, "   [OK]");
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "2. 创建GATT服务...");
//     if (!wireless_ble_create_gatt_service()) { 
//         ESP_LOGE(TAG, "   [FAIL]"); 
//         return; 
//     }
//     ESP_LOGI(TAG, "   [OK]");
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "3. 启动GATT服务...");
//     if (!wireless_ble_start_gatt_service()) { 
//         ESP_LOGE(TAG, "   [FAIL]"); 
//         return; 
//     }
//     ESP_LOGI(TAG, "   [OK]");
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "4. 查询本机MAC地址...");
//     wireless_send_at_command("AT+BLEADDR?");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//     ESP_LOGI(TAG, "   请记录MAC地址供客户端使用");
    
//     ESP_LOGI(TAG, "5. 设置设备名称...");
//     wireless_ble_set_name(BLE_SERVER_NAME);
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "6. 设置广播参数...");
//     wireless_ble_set_adv_param(50, 50);
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "7. 设置广播数据...");
//     wireless_ble_set_adv_data(BLE_SERVER_ADV_DATA);
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "8. 开始广播...");
//     if (!wireless_ble_start_adv()) { 
//         ESP_LOGE(TAG, "   [FAIL]"); 
//         return; 
//     }
//     ESP_LOGI(TAG, "   [OK] 等待客户端连接...");
    
//     // 等待连接（这里简化处理，实际应用中需要检测连接状态）
//     ESP_LOGI(TAG, "等待客户端连接（30秒）...");
//     vTaskDelay(pdMS_TO_TICKS(30000));
    
//     // 配置SPP
//     ESP_LOGI(TAG, "9. 配置SPP参数...");
//     if (!wireless_ble_config_spp(SPP_SERVER_TX_SRV, SPP_SERVER_TX_CHAR,
//                                   SPP_SERVER_RX_SRV, SPP_SERVER_RX_CHAR, SPP_MTU_SIZE)) {
//         ESP_LOGE(TAG, "   [FAIL]");
//         return;
//     }
//     ESP_LOGI(TAG, "   [OK]");
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "10. 进入SPP透传...");
//     if (!wireless_ble_enter_spp()) { 
//         ESP_LOGE(TAG, "   [FAIL]"); 
//         return; 
//     }
//     ESP_LOGI(TAG, "   [OK] 已进入透传模式");
    
//     // 发送测试数据
//     ESP_LOGI(TAG, "11. 发送测试数据...");
//     for (int i = 0; i < 5; i++) {
//         char buf[32];
//         snprintf(buf, sizeof(buf), "Server data #%d\r\n", i);
//         wireless_send_string(buf);
//         ESP_LOGI(TAG, "   发送: %s", buf);
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
    
//     vTaskDelay(pdMS_TO_TICKS(5000));
    
//     ESP_LOGI(TAG, "12. 退出透传...");
//     wireless_ble_exit_spp();
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     ESP_LOGI(TAG, "测试完成\n");
// }

// /**
//  * @brief 测试7：BLE SPP客户端
//  */
// void test_ble_spp_client(void)
// {
//     ESP_LOGI(TAG, "========== 测试7：BLE SPP客户端 ==========");
    
//     ESP_LOGI(TAG, "1. 初始化蓝牙(Client模式)...");
//     if (!wireless_ble_init(1)) { 
//         ESP_LOGE(TAG, "   [FAIL]"); 
//         return; 
//     }
//     ESP_LOGI(TAG, "   [OK]");
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "2. 扫描蓝牙设备(3秒)...");
//     wireless_ble_scan(1, 3);
//     vTaskDelay(pdMS_TO_TICKS(4000));
    
//     ESP_LOGI(TAG, "3. 连接服务端: %s...", BLE_SERVER_MAC);
//     ESP_LOGI(TAG, "   (等待10-30秒)");
//     if (!wireless_ble_connect(0, BLE_SERVER_MAC)) {
//         ESP_LOGE(TAG, "   [FAIL] 请检查服务端MAC地址");
//         return;
//     }
//     ESP_LOGI(TAG, "   [OK]");
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     ESP_LOGI(TAG, "4. 查询连接状态...");
//     wireless_send_at_command("AT+BLECONN?");
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     ESP_LOGI(TAG, "5. 配置SPP参数...");
//     if (!wireless_ble_config_spp(SPP_CLIENT_TX_SRV, SPP_CLIENT_TX_CHAR,
//                                   SPP_CLIENT_RX_SRV, SPP_CLIENT_RX_CHAR, SPP_MTU_SIZE + 2)) {
//         ESP_LOGE(TAG, "   [FAIL]");
//         return;
//     }
//     ESP_LOGI(TAG, "   [OK]");
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     ESP_LOGI(TAG, "6. 进入SPP透传...");
//     if (!wireless_ble_enter_spp()) { 
//         ESP_LOGE(TAG, "   [FAIL]"); 
//         return; 
//     }
//     ESP_LOGI(TAG, "   [OK] 已进入透传模式");
    
//     ESP_LOGI(TAG, "7. 发送测试数据...");
//     for (int i = 0; i < 5; i++) {
//         char buf[32];
//         snprintf(buf, sizeof(buf), "Client data #%d\r\n", i);
//         wireless_send_string(buf);
//         ESP_LOGI(TAG, "   发送: %s", buf);
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
    
//     vTaskDelay(pdMS_TO_TICKS(5000));
    
//     ESP_LOGI(TAG, "8. 退出透传...");
//     wireless_ble_exit_spp();
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     ESP_LOGI(TAG, "9. 保存透传配置（断电自动重连）...");
//     if (wireless_ble_save_trans_link(1, SPP_CLIENT_TX_SRV, SPP_CLIENT_TX_CHAR,
//                                       SPP_CLIENT_RX_SRV, SPP_CLIENT_RX_CHAR, BLE_SERVER_MAC)) {
//         ESP_LOGI(TAG, "   [OK] 配置已保存，重启后自动连接");
//     } else {
//         ESP_LOGE(TAG, "   [FAIL]");
//     }
    
//     ESP_LOGI(TAG, "测试完成\n");
// }

// // ========================================
// // 主函数
// // ========================================

// void app_main(void)
// {
//     ESP_LOGI(TAG, "================================================");
//     ESP_LOGI(TAG, "       ESP32无线通信模块测试程序");
//     ESP_LOGI(TAG, "================================================");
//     ESP_LOGI(TAG, "");
//     ESP_LOGI(TAG, "测试项目：");
//     ESP_LOGI(TAG, "  1 - 基础AT指令测试");
//     ESP_LOGI(TAG, "  2 - WiFi连接测试");
//     ESP_LOGI(TAG, "  3 - TCP客户端测试");
//     ESP_LOGI(TAG, "  4 - TCP透传测试");
//     ESP_LOGI(TAG, "  5 - 蓝牙扫描测试");
//     ESP_LOGI(TAG, "  6 - BLE SPP服务端测试");
//     ESP_LOGI(TAG, "  7 - BLE SPP客户端测试");
//     ESP_LOGI(TAG, "");
    
//     // 初始化无线模块
//     wireless_init();
    
//     // 等待模块稳定
//     ESP_LOGI(TAG, "等待模块稳定...");
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     // 选择要运行的测试（修改这里选择测试项目）
//     int test_mode = 1;  // 默认运行测试1
    
//     ESP_LOGI(TAG, "运行测试 %d...\n", test_mode);
    
//     switch (test_mode) {
//         case 1:
//             test_at_basic();
//             break;
//         case 2:
//             test_wifi_connect();
//             break;
//         case 3:
//             test_tcp_client();
//             break;
//         case 4:
//             test_tcp_transparent();
//             break;
//         case 5:
//             test_ble_scan();
//             break;
//         case 6:
//             test_ble_spp_server();
//             break;
//         case 7:
//             test_ble_spp_client();
//             break;
//         default:
//             ESP_LOGE(TAG, "无效的测试项目");
//             break;
//     }
    
//     ESP_LOGI(TAG, "所有测试完成");
// }
