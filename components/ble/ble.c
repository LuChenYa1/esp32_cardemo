/**
 * @file ble.c
 * @brief 蓝牙低功耗(BLE)模块实现
 * 
 * 功能：
 * - BLE GATT服务器
 * - BLE广播
 * - 数据收发
 * 
 * 注意：
 * - 当前代码被注释，功能未实现
 * - BLE与经典蓝牙不能同时使用
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <inttypes.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
// #include "esp_system.h"
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include "esp_bt.h"

// #include "esp_gap_ble_api.h"
// #include "esp_gatts_api.h"
// #include "esp_bt_defs.h"
// #include "esp_bt_main.h"
// #include "esp_bt_device.h"
// #include "esp_gatt_common_api.h"

// #include "sdkconfig.h"

// #define GATTS_TAG "GATTS_DEMO"

// ///Declare the static function
// static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
// static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

// /**
//  * @brief GATT服务和特征UUID定义
//  * 
//  * 定义了两个GATT服务配置文件的UUID和服务相关信息，用于标识BLE设备提供的服务和特征
//  */
// #define GATTS_SERVICE_UUID_TEST_A   0x00FF      // Profile A的16位服务UUID
// #define GATTS_CHAR_UUID_TEST_A      0xFF01      // Profile A的16位特征UUID
// #define GATTS_DESCR_UUID_TEST_A     0x3333      // Profile A的16位描述符UUID
// #define GATTS_NUM_HANDLE_TEST_A     4           // Profile A所需句柄数量

// #define GATTS_SERVICE_UUID_TEST_B   0x00EE      // Profile B的16位服务UUID
// #define GATTS_CHAR_UUID_TEST_B      0xEE01      // Profile B的16位特征UUID
// #define GATTS_DESCR_UUID_TEST_B     0x2222      // Profile B的16位描述符UUID
// #define GATTS_NUM_HANDLE_TEST_B     4           // Profile B所需句柄数量

// /**
//  * @brief 全局变量和常量定义
//  * 
//  * 定义了BLE GATT服务器示例程序中使用的各种全局变量和常量，包括设备名称、
//  * 特征值长度限制、准备写缓冲区大小、初始特征值、描述符值、MTU值和可读特征值等
//  */
// static char test_device_name[ESP_BLE_ADV_NAME_LEN_MAX] = "ESP_GATTS_DEMO";  // 测试设备名称，用于BLE广播中显示

// #define TEST_MANUFACTURER_DATA_LEN  17                                      // 厂商数据长度定义

// #define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40                                   // GATT特征值最大长度(64字节)

// #define PREPARE_BUF_MAX_SIZE 1024                                          // 预准备写入缓冲区最大大小

// static uint8_t char1_str[] = {0x11,0x22,0x33};                           // 特征值初始数据(3字节)

// static uint16_t descr_value = 0x0;                                        // 描述符当前值，用于控制通知/指示使能状态

// static uint16_t local_mtu = 23;                                           // 本地MTU值，默认23字节(ATT_MTU最小值)

// static uint8_t char_value_read[CONFIG_EXAMPLE_CHAR_READ_DATA_LEN] = {0xDE,0xED,0xBE,0xEF}; // 可读特征值数据(4字节)


// static esp_gatt_char_prop_t a_property = 0;
// static esp_gatt_char_prop_t b_property = 0;

// /**
//  * @brief GATT特征值属性结构体
//  * 
//  * 定义了GATT特征(characteristic)的默认值和属性参数，包括最大长度、
//  * 实际长度和初始值。此结构体用于在创建GATT服务时初始化特征值
//  */
// static esp_attr_value_t gatts_demo_char1_val =
// {
//     .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,  // 特征值的最大长度，定义为0x40(64字节)
//     .attr_len     = sizeof(char1_str),            // 特征值的实际长度，等于char1_str数组的大小(3字节)
//     .attr_value   = char1_str,                    // 指向特征值初始数据的指针，指向{0x11,0x22,0x33}
// };

// /**
//  * @brief 广告配置完成状态变量及相关标志位
//  * 
//  * 用于跟踪广告数据和扫描响应数据的配置状态，确保在配置完成后才开始广告
//  */
// static uint8_t adv_config_done = 0;          // 广告配置完成状态，位掩码形式，bit0=广告数据配置状态，bit1=扫描响应数据配置状态
// #define adv_config_flag      (1 << 0)        // 广告数据配置完成标志位
// #define scan_rsp_config_flag (1 << 1)        // 扫描响应数据配置完成标志位

// #ifdef CONFIG_EXAMPLE_SET_RAW_ADV_DATA
// static uint8_t raw_adv_data[] = {
//     /* Flags */
//     0x02, ESP_BLE_AD_TYPE_FLAG, 0x06,               // Length 2, Data Type ESP_BLE_AD_TYPE_FLAG, Data 1 (LE General Discoverable Mode, BR/EDR Not Supported)
//     /* TX Power Level */
//     0x02, ESP_BLE_AD_TYPE_TX_PWR, 0xEB,             // Length 2, Data Type ESP_BLE_AD_TYPE_TX_PWR, Data 2 (-21)
//     /* Complete 16-bit Service UUIDs */
//     0x03, ESP_BLE_AD_TYPE_16SRV_CMPL, 0xAB, 0xCD    // Length 3, Data Type ESP_BLE_AD_TYPE_16SRV_CMPL, Data 3 (UUID)
// };

// static uint8_t raw_scan_rsp_data[] = {
//     /* Complete Local Name */
//     0x0F, ESP_BLE_AD_TYPE_NAME_CMPL, 'E', 'S', 'P', '_', 'G', 'A', 'T', 'T', 'S', '_', 'D', 'E', 'M', 'O'   // Length 15, Data Type ESP_BLE_AD_TYPE_NAME_CMPL, Data (ESP_GATTS_DEMO)
// };
// #else

// /**
//  * @brief 128位服务UUID数组
//  * 
//  * 此数组定义了两个128位的蓝牙服务UUID，用于在BLE广告数据中标识设备支持的服务
//  * 第一个UUID在字节12-13位置包含16位UUID值0xEE00
//  * 第二个UUID在字节12-15位置包含32位UUID值0xFF000000
//  * 
//  * UUID格式遵循蓝牙标准，采用小端序(LSB)格式存储
//  */
// static uint8_t adv_service_uuid128[32] = {
//     /* LSB <--------------------------------------------------------------------------------> MSB */
//     //第一个UUID，16位，[12]，[13]是值
//     0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
//     //第二个UUID，32位，[12]，[13]，[14]，[15]是值
//     0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
// };


// /**
//  * @brief 定义BLE广播数据结构
//  * 
//  * 此结构体配置了BLE设备的广播参数，包括设备名称、连接间隔、服务UUID等信息，
//  * 用于向周围的BLE中央设备广播此外围设备的存在和基本特性
//  */
// static esp_ble_adv_data_t adv_data = {
//     .set_scan_rsp = false,                    // 广播数据类型，false表示这是广播数据而非扫描响应数据
//     .include_name = true,                     // 是否在广播数据中包含设备名称
//     .include_txpower = false,                 // 是否在广播数据中包含发射功率信息
//     .min_interval = 0x0006,                   // 从设备连接最小间隔，单位1.25毫秒，此处为7.5毫秒
//     .max_interval = 0x0010,                   // 从设备连接最大间隔，单位1.25毫秒，此处为20毫秒
//     .appearance = 0x00,                       // 设备外观类别，0x00表示未指定外观
//     .manufacturer_len = 0,                    // 厂商特定数据长度，当前禁用(原值TEST_MANUFACTURER_DATA_LEN)
//     .p_manufacturer_data = NULL,              // 指向厂商特定数据的指针，当前禁用(原值&test_manufacturer[0])
//     .service_data_len = 0,                    // 服务数据长度
//     .p_service_data = NULL,                   // 指向服务数据的指针
//     .service_uuid_len = sizeof(adv_service_uuid128), // 服务UUID数据长度
//     .p_service_uuid = adv_service_uuid128,    // 指向服务UUID数据的指针
//     .flag = (ESP_BLE_ADV_FLAG_GEN_DISC |      // 广播标志位，包含通用可发现模式和不支持BR/EDR模式
//              ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
// };

// /**
//  * @brief 定义BLE扫描响应数据结构
//  * 
//  * 此结构体配置了BLE设备的扫描响应数据，当中央设备扫描此外围设备时会收到这些信息，
//  * 包含设备名称和发射功率等信息，帮助中央设备更好地识别此外围设备
//  */
// static esp_ble_adv_data_t scan_rsp_data = {
//     .set_scan_rsp = true,                     // 标识这是扫描响应数据而不是广播数据
//     .include_name = true,                     // 在扫描响应中包含设备名称
//     .include_txpower = true,                  // 在扫描响应中包含发射功率信息
//     //.min_interval = 0x0006,                 // 注释掉的连接间隔参数（不影响扫描响应）
//     //.max_interval = 0x0010,                 // 注释掉的连接间隔参数（不影响扫描响应）
//     .appearance = 0x00,                       // 设备外观类别，0x00表示未指定外观
//     .manufacturer_len = 0,                    // 厂商特定数据长度，当前禁用(原值TEST_MANUFACTURER_DATA_LEN)
//     .p_manufacturer_data = NULL,              // 指向厂商特定数据的指针，当前禁用(原值&test_manufacturer[0])
//     .service_data_len = 0,                    // 服务数据长度
//     .p_service_data = NULL,                   // 指向服务数据的指针
//     .service_uuid_len = sizeof(adv_service_uuid128), // 服务UUID数据长度
//     .p_service_uuid = adv_service_uuid128,    // 指向服务UUID数据的指针
//     .flag = (ESP_BLE_ADV_FLAG_GEN_DISC |      // 广播标志位，包含通用可发现模式和不支持BR/EDR模式
//              ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
// };

// #endif /* CONFIG_SET_RAW_ADV_DATA */

// /**
//  * @brief 定义BLE广告参数结构
//  * 
//  * 此结构体配置了BLE设备的广告参数，控制设备如何广播自己，包括广告间隔、
//  * 广告类型、地址类型、信道映射和过滤策略等参数
//  */
// static esp_ble_adv_params_t adv_params = {
//     .adv_int_min        = 0x20,              // 最小广告间隔，0x20 * 0.625ms = 32ms
//     .adv_int_max        = 0x40,              // 最大广告间隔，0x40 * 0.625ms = 64ms
//     .adv_type           = ADV_TYPE_IND,      // 广告类型，可连接的通用广告类型
//     .own_addr_type      = BLE_ADDR_TYPE_PUBLIC, // 使用公共设备地址
//     //.peer_addr            =              // 对端设备地址（未使用，因为是广播给任何设备）
//     //.peer_addr_type       =              // 对端设备地址类型（未使用）
//     .channel_map        = ADV_CHNL_ALL,      // 在所有三个广告信道(37, 38, 39)上广播
//     .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // 允许任何设备扫描和连接
// };
// /**
//  * @brief 定义GATT服务配置文件数量和ID
//  * 
//  * 这些宏定义了BLE GATT服务器中使用的配置文件数量及各个配置文件的应用ID
//  * 用于区分不同的GATT服务配置文件，每个配置文件可以管理独立的服务和特征
//  */
// #define PROFILE_NUM 2                        // 定义GATT服务配置文件总数为2个
// #define PROFILE_A_APP_ID 0                   // 定义配置文件A的应用ID为0
// #define PROFILE_B_APP_ID 1                   // 定义配置文件B的应用ID为1
// /**
//  * @brief GATT服务配置文件实例结构体
//  * 
//  * 此结构体用于存储GATT服务配置文件的相关信息，包括回调函数、接口ID、
//  * 应用ID、连接ID、服务句柄、特征句柄、描述符句柄以及相关的UUID和权限信息
//  */
// struct gatts_profile_inst {
//     esp_gatts_cb_t gatts_cb;                 // GATT服务事件回调函数指针
//     uint16_t gatts_if;                       // GATT接口ID，用于区分不同的GATT接口
//     uint16_t app_id;                         // 应用程序ID，用于标识不同的GATT应用程序
//     uint16_t conn_id;                        // 连接ID，标识当前的BLE连接
//     uint16_t service_handle;                 // 服务句柄，用于标识GATT服务
//     esp_gatt_srvc_id_t service_id;           // 服务ID结构体，包含服务的唯一标识信息
//     uint16_t char_handle;                    // 特征句柄，用于标识GATT特征
//     esp_bt_uuid_t char_uuid;                 // 特征UUID，标识特征的类型
//     esp_gatt_perm_t perm;                    // 特征访问权限
//     esp_gatt_char_prop_t property;           // 特征属性，如读、写、通知等
//     uint16_t descr_handle;                   // 描述符句柄，用于标识GATT描述符
//     esp_bt_uuid_t descr_uuid;                // 描述符UUID，标识描述符的类型
// };

// /**
//  * @brief GATT服务配置文件实例数组
//  * 
//  * 该数组用于存储多个GATT服务配置文件实例，每个配置文件包含其对应的回调函数、
//  * GATT接口ID和其他状态信息。在ESP32的BLE GATT服务器实现中，每个服务配置文件
//  * 可以管理一个独立的GATT服务及其特征和描述符
//  * 
//  * 数组中的每个元素对应一个GATT服务配置文件，初始化时设置了各自的事件回调函数
//  * 和初始状态（如gatts_if设为ESP_GATT_IF_NONE表示尚未获得有效的GATT接口ID）
//  */
// static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
//     [PROFILE_A_APP_ID] = {                             // 配置文件A（应用ID为0）
//         .gatts_cb = gatts_profile_a_event_handler,    // 设置配置文件A的事件回调函数
//         .gatts_if = ESP_GATT_IF_NONE,                 /* 初始时没有获取到gatt_if，所以设置为ESP_GATT_IF_NONE */
//         .conn_id = 0xFFFF,                            /* 初始连接ID为无效值，表示未连接 */
//     },
//     [PROFILE_B_APP_ID] = {                             // 配置文件B（应用ID为1）
//         .gatts_cb = gatts_profile_b_event_handler,    /* 设置配置文件B的事件回调函数，此演示中未完全实现，功能类似配置文件A */
//         .gatts_if = ESP_GATT_IF_NONE,                 /* 初始时没有获取到gatt_if，所以设置为ESP_GATT_IF_NONE */
//         .conn_id = 0xFFFF,                            /* 初始连接ID为无效值，表示未连接 */
//     },
// };

// /**
//  * @brief 预写入操作环境结构体
//  * 
//  * 用于处理GATT预写入请求的临时缓冲区，支持长特征值的分段写入操作
//  */
// typedef struct {
//     uint8_t                 *prepare_buf;             // 指向预写入缓冲区的指针
//     int                     prepare_len;              // 预写入数据的实际长度
// } prepare_type_env_t;

// /**
//  * @brief 配置文件A和B的预写入环境变量
//  * 
//  * 分别为两个GATT配置文件维护预写入操作的缓冲区和状态
//  */
// static prepare_type_env_t a_prepare_write_env;        // 配置文件A的预写入环境
// static prepare_type_env_t b_prepare_write_env;        // 配置文件B的预写入环境

// /**
//  * @brief 处理GATT写入事件环境
//  * 
//  * 管理预写入请求的缓冲区分配和数据存储
//  * 
//  * @param gatts_if GATT接口ID
//  * @param prepare_write_env 预写入环境结构体指针
//  * @param param GATT服务器事件参数
//  */
// void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

// /**
//  * @brief 处理GATT执行写入事件环境
//  * 
//  * 处理预写入请求的最终执行，包括数据发送和缓冲区清理
//  * 
//  * @param prepare_write_env 预写入环境结构体指针
//  * @param param GATT服务器事件参数
//  */
// void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
// /**
//  * @brief GAP事件处理函数，用于处理BLE GAP层的各种事件
//  * 
//  * 该函数负责处理蓝牙低功耗GAP（Generic Access Profile）层产生的各种事件，
//  * 包括广告数据设置完成、扫描响应数据设置完成、广告启动/停止完成、连接参数更新等
//  * 
//  * @param event GAP事件类型，标识具体的事件
//  * @param param 指向事件参数结构体的指针，包含事件相关的详细信息
//  * @return 无返回值
//  */
// static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
// {
//     switch (event) {
// #ifdef CONFIG_EXAMPLE_SET_RAW_ADV_DATA
//     // 处理原始广告数据设置完成事件
//     case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
//         adv_config_done &= (~adv_config_flag);
//         if (adv_config_done==0){
//             esp_ble_gap_start_advertising(&adv_params);
//         }
//         break;
//     // 处理原始扫描响应数据设置完成事件
//     case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
//         adv_config_done &= (~scan_rsp_config_flag);
//         if (adv_config_done==0){
//             esp_ble_gap_start_advertising(&adv_params);
//         }
//         break;
// #else
//     // 处理广告数据设置完成事件
//     case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
//         adv_config_done &= (~adv_config_flag);
//         if (adv_config_done == 0){
//             esp_ble_gap_start_advertising(&adv_params);
//         }
//         break;
//     // 处理扫描响应数据设置完成事件
//     case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
//         adv_config_done &= (~scan_rsp_config_flag);
//         if (adv_config_done == 0){
//             esp_ble_gap_start_advertising(&adv_params);
//         }
//         break;
// #endif
//     // 处理广告启动完成事件
//     case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
//         //advertising start complete event to indicate advertising start successfully or failed
//         if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
//             ESP_LOGE(GATTS_TAG, "广告启动失败, 状态 %d", param->adv_start_cmpl.status);
//             break;
//         }
//         ESP_LOGI(GATTS_TAG, "广告启动成功");
//         break;
//     // 处理广告停止完成事件
//     case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
//         if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
//             ESP_LOGE(GATTS_TAG, "广告停止失败, 状态 %d", param->adv_stop_cmpl.status);
//             break;
//         }
//         ESP_LOGI(GATTS_TAG, "广告停止成功");
//         break;
//     // 处理连接参数更新事件
//     case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
//          ESP_LOGI(GATTS_TAG, "连接参数更新, 状态 %d, 连接间隔 %d, 延迟 %d, 超时 %d",
//                   param->update_conn_params.status,
//                   param->update_conn_params.conn_int,
//                   param->update_conn_params.latency,
//                   param->update_conn_params.timeout);
//         break;
//     // 处理数据包长度更新完成事件
//     case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
//         ESP_LOGI(GATTS_TAG, "数据包长度更新, 状态 %d, 接收 %d, 发送 %d",
//                   param->pkt_data_length_cmpl.status,
//                   param->pkt_data_length_cmpl.params.rx_len,
//                   param->pkt_data_length_cmpl.params.tx_len);
//         break;
//     default:
//         break;
//     }
// }

// /**
//  * @brief 处理GATT服务器写事件环境
//  * 
//  * 该函数处理BLE GATT服务器的写请求，包括准备写和普通写两种类型，
//  * 验证偏移量和长度的有效性，并在需要响应时发送适当的GATT响应。
//  * 
//  * @param gatts_if GATT服务器接口句柄
//  * @param prepare_write_env 准备写操作的环境结构体指针，用于存储准备写的数据缓冲区和长度
//  * @param param BLE GATT服务器回调参数结构体指针，包含写操作的具体参数信息
//  * 
//  * @return void 无返回值
//  */
// void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
//     esp_gatt_status_t status = ESP_GATT_OK;
//     if (param->write.need_rsp){
//         if (param->write.is_prep) {
//             // 验证准备写操作的偏移量和长度是否有效
//             if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
//                 status = ESP_GATT_INVALID_OFFSET;
//             } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
//                 status = ESP_GATT_INVALID_ATTR_LEN;
//             }
//             // 检查并初始化准备写缓冲区
//             if (status == ESP_GATT_OK && prepare_write_env->prepare_buf == NULL) {
//                 prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
//                 prepare_write_env->prepare_len = 0;
//                 if (prepare_write_env->prepare_buf == NULL) {
//                     ESP_LOGE(GATTS_TAG, "Gatt_server 准备写入内存不足");
//                     status = ESP_GATT_NO_RESOURCES;
//                 }
//             }

//             // 安全修复：使用calloc确保内存被零初始化
//             esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)calloc(1, sizeof(esp_gatt_rsp_t));
//             if (gatt_rsp) {
//                 gatt_rsp->attr_value.len = param->write.len;
//                 gatt_rsp->attr_value.handle = param->write.handle;
//                 gatt_rsp->attr_value.offset = param->write.offset;
//                 gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
//                 memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
//                 esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
//                 if (response_err != ESP_OK){
//                     ESP_LOGE(GATTS_TAG, "发送响应错误\n");
//                 }
//                 free(gatt_rsp);
//             } else {
//                 ESP_LOGE(GATTS_TAG, "分配malloc失败，无法发送响应错误\n");
//                 status = ESP_GATT_NO_RESOURCES;
//             }
//             if (status != ESP_GATT_OK){
//                 return;
//             }
//             // 将写入的数据复制到准备缓冲区中
//             memcpy(prepare_write_env->prepare_buf + param->write.offset,
//                    param->write.value,
//                    param->write.len);
//             prepare_write_env->prepare_len += param->write.len;

//         }else{
//             // 发送普通写操作的响应
//             esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
//         }
//     }
// }

// /**
//  * @brief 处理GATT服务器执行写入事件的环境清理函数
//  * 
//  * 该函数处理BLE GATT服务器中的执行写入事件，根据执行标志决定是处理预写入数据还是取消准备写入，
//  * 并清理相关的缓冲区资源。
//  * 
//  * @param prepare_write_env 指向准备写入环境结构体的指针，包含预写入的缓冲区和长度信息
//  * @param param 指向BLE GATT服务器回调参数结构体的指针，包含执行写入的相关参数
//  * 
//  * @return 无返回值
//  */
// void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
//     if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC){
//         ESP_LOG_BUFFER_HEX(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
//     }else{
//         ESP_LOGI(GATTS_TAG,"准备写入取消");
//     }
//     // 释放预写入缓冲区内存并重置相关变量
//     if (prepare_write_env->prepare_buf) {
//         free(prepare_write_env->prepare_buf);
//         prepare_write_env->prepare_buf = NULL;
//     }
//     prepare_write_env->prepare_len = 0;
// }

// /**
//  * @brief GATT Server Profile A 事件处理函数
//  * 
//  * 处理BLE GATT服务器Profile A的各种事件，包括服务注册、特征读写、连接管理等
//  * 
//  * @param event GATT服务器回调事件类型
//  * @param gatts_if GATT服务器接口句柄
//  * @param param 指向事件参数结构体的指针，包含具体事件相关数据
//  */
// static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
//     switch (event) {
//     case ESP_GATTS_REG_EVT:
//         ESP_LOGI(GATTS_TAG, "GATT服务器注册, 状态 %d, 应用ID %d, GATT接口 %d", param->reg.status, param->reg.app_id, gatts_if);
//         gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
//         gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
//         gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
//         gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_A;

//         esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(test_device_name);
//         if (set_dev_name_ret){
//             ESP_LOGE(GATTS_TAG, "设置设备名称失败, 错误代码 = %x", set_dev_name_ret);
//         }
// #ifdef CONFIG_EXAMPLE_SET_RAW_ADV_DATA
//         esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
//         if (raw_adv_ret){
//             ESP_LOGE(GATTS_TAG, "配置原始广告数据失败, 错误代码 = %x ", raw_adv_ret);
//         }
//         adv_config_done |= adv_config_flag;
//         esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
//         if (raw_scan_ret){
//             ESP_LOGE(GATTS_TAG, "配置原始扫描响应数据失败, 错误代码 = %x", raw_scan_ret);
//         }
//         adv_config_done |= scan_rsp_config_flag;
// #else
//         // 配置广播数据
//         esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
//         if (ret){
//             ESP_LOGE(GATTS_TAG, "配置广播数据失败, 错误代码 = %x", ret);
//         }
//         adv_config_done |= adv_config_flag;
//         // 配置扫描响应数据
//         ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
//         if (ret){
//             ESP_LOGE(GATTS_TAG, "配置扫描响应数据失败, 错误代码 = %x", ret);
//         }
//         adv_config_done |= scan_rsp_config_flag;

// #endif
//         esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
//         break;
//     case ESP_GATTS_READ_EVT: {
//         ESP_LOGI(GATTS_TAG,
//                     "特征读取请求: conn_id=%d, trans_id=%" PRIu32 ", handle=%d, is_long=%d, offset=%d, need_rsp=%d",
//                     param->read.conn_id, param->read.trans_id, param->read.handle,
//                     param->read.is_long, param->read.offset, param->read.need_rsp);

//         // 如果不需要响应，提前退出（栈自动处理）
//         if (!param->read.need_rsp) {
//             return;
//         }

//         esp_gatt_rsp_t rsp;
//         memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
//         rsp.attr_value.handle = param->read.handle;

//         // 处理描述符读取请求
//         if (param->read.handle == gl_profile_tab[PROFILE_A_APP_ID].descr_handle) {
//             memcpy(rsp.attr_value.value, &descr_value, 2);
//             rsp.attr_value.len = 2;
//             esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
//             return;
//         }

//         // 处理特征读取请求
//         if (param->read.handle == gl_profile_tab[PROFILE_A_APP_ID].char_handle) {
//             uint16_t offset = param->read.offset;

//             // 验证读取偏移量
//             if (param->read.is_long && offset > CONFIG_EXAMPLE_CHAR_READ_DATA_LEN) {
//                 ESP_LOGW(GATTS_TAG, "Read offset (%d) out of range (0-%d)", offset, CONFIG_EXAMPLE_CHAR_READ_DATA_LEN);
//                 rsp.attr_value.len = 0;
//                 esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_INVALID_OFFSET, &rsp);
//                 return;
//             }

//             // 根据MTU确定响应长度
//             uint16_t mtu_size = local_mtu - 1;  // ATT头部(1字节)
//             uint16_t send_len = (CONFIG_EXAMPLE_CHAR_READ_DATA_LEN - offset > mtu_size) ? mtu_size : (CONFIG_EXAMPLE_CHAR_READ_DATA_LEN - offset);

//             memcpy(rsp.attr_value.value, &char_value_read[offset], send_len);
//             rsp.attr_value.len = send_len;

//             // 发送响应到GATT客户端
//             esp_err_t err = esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
//             if (err != ESP_OK) {
//                 ESP_LOGE(GATTS_TAG, "Failed to send response: %s", esp_err_to_name(err));
//             }
//         }
//         break;
//     }
//     case ESP_GATTS_WRITE_EVT: {
//         ESP_LOGI(GATTS_TAG, "特征写入, 连接ID %d, 事务ID %" PRIu32 ", 句柄 %d", param->write.conn_id, param->write.trans_id, param->write.handle);
//         if (!param->write.is_prep){
//             ESP_LOGI(GATTS_TAG, "值长度 %d, 值 ", param->write.len);
//             ESP_LOG_BUFFER_HEX(GATTS_TAG, param->write.value, param->write.len);
//             if (gl_profile_tab[PROFILE_A_APP_ID].descr_handle == param->write.handle && param->write.len == 2){
//                 descr_value = param->write.value[1]<<8 | param->write.value[0];
//                 if (descr_value == 0x0001){
//                     if (a_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
//                         ESP_LOGI(GATTS_TAG, "通知启用");
//                         uint8_t notify_data[15];
//                         for (int i = 0; i < sizeof(notify_data); ++i)
//                         {
//                             notify_data[i] = i%0xff;
//                         }
//                         //the size of notify_data[] need less than MTU size
//                         esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle,
//                                                 sizeof(notify_data), notify_data, false);
//                     }
//                 }else if (descr_value == 0x0002){
//                     if (a_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
//                         ESP_LOGI(GATTS_TAG, "指示启用");
//                         uint8_t indicate_data[15];
//                         for (int i = 0; i < sizeof(indicate_data); ++i)
//                         {
//                             indicate_data[i] = i%0xff;
//                         }
//                         //the size of indicate_data[] need less than MTU size
//                         esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle,
//                                                 sizeof(indicate_data), indicate_data, true);
//                     }
//                 }
//                 else if (descr_value == 0x0000){
//                     ESP_LOGI(GATTS_TAG, "通知/指示禁用");
//                 }else{
//                     ESP_LOGE(GATTS_TAG, "未知的描述符值");
//                     ESP_LOG_BUFFER_HEX(GATTS_TAG, param->write.value, param->write.len);
//                 }

//             }
//         }
//         example_write_event_env(gatts_if, &a_prepare_write_env, param);
//         break;
//     }
//     case ESP_GATTS_EXEC_WRITE_EVT:
//         ESP_LOGI(GATTS_TAG,"执行写入事件");
//         esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
//         example_exec_write_event_env(&a_prepare_write_env, param);
//         break;
//     case ESP_GATTS_MTU_EVT:
//         ESP_LOGI(GATTS_TAG, "MTU交换, MTU %d", param->mtu.mtu);
//         local_mtu = param->mtu.mtu;
//         break;
//     case ESP_GATTS_UNREG_EVT:
//         break;
//     case ESP_GATTS_CREATE_EVT:
//         ESP_LOGI(GATTS_TAG, "服务创建, 状态 %d, 服务句柄 %d", param->create.status, param->create.service_handle);
//         gl_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
//         gl_profile_tab[PROFILE_A_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
//         gl_profile_tab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_A;

//         esp_ble_gatts_start_service(gl_profile_tab[PROFILE_A_APP_ID].service_handle);
//         a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
//         esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].char_uuid,
//                                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
//                                                         a_property,
//                                                         &gatts_demo_char1_val, NULL);
//         if (add_char_ret){
//             ESP_LOGE(GATTS_TAG, "添加特征失败, 错误代码 =%x",add_char_ret);
//         }
//         break;
//     case ESP_GATTS_ADD_INCL_SRVC_EVT:
//         break;
//     case ESP_GATTS_ADD_CHAR_EVT: {
//         uint16_t length = 0;
//         const uint8_t *prf_char;

//         ESP_LOGI(GATTS_TAG, "特征添加, 状态 %d, 属性句柄 %d, 服务句柄 %d",
//                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
//         gl_profile_tab[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
//         gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
//         gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
//         esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);
//         if (get_attr_ret == ESP_FAIL){
//             ESP_LOGE(GATTS_TAG, "非法句柄");
//         }

//         ESP_LOGI(GATTS_TAG, "特征长度 = %x", length);
//         for(int i = 0; i < length; i++){
//             ESP_LOGI(GATTS_TAG, "prf_char[%x] =%x",i,prf_char[i]);
//         }
//         esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].descr_uuid,
//                                                                 ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
//         if (add_descr_ret){
//             ESP_LOGE(GATTS_TAG, "添加特征描述失败, 错误代码 =%x", add_descr_ret);
//         }
//         break;
//     }
//     case ESP_GATTS_ADD_CHAR_DESCR_EVT:
//         gl_profile_tab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
//         ESP_LOGI(GATTS_TAG, "描述符添加, 状态 %d, 属性句柄 %d, 服务句柄 %d",
//                  param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
//         break;
//     case ESP_GATTS_DELETE_EVT:
//         break;
//     case ESP_GATTS_START_EVT:
//         ESP_LOGI(GATTS_TAG, "服务启动, 状态 %d, 服务句柄 %d",
//                  param->start.status, param->start.service_handle);
//         break;
//     case ESP_GATTS_STOP_EVT:
//         break;
//     case ESP_GATTS_CONNECT_EVT: {
//         esp_ble_conn_update_params_t conn_params = {0};
//         memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
//         /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
//         conn_params.latency = 0;
//         conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
//         conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
//         conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
//         ESP_LOGI(GATTS_TAG, "已连接, conn_id %u, 远程设备 "ESP_BD_ADDR_STR"",
//                  param->connect.conn_id, ESP_BD_ADDR_HEX(param->connect.remote_bda));
//         gl_profile_tab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;
//         ESP_LOGI(GATTS_TAG, "A 连接成功 conn_id=%d", param->connect.conn_id);
//         //start sent the update connection parameters to the peer device.
//         esp_ble_gap_update_conn_params(&conn_params);
//         break;
//     }
//     case ESP_GATTS_DISCONNECT_EVT:
//         ESP_LOGI(GATTS_TAG, "已断开连接, 远程设备 "ESP_BD_ADDR_STR", 原因 0x%02x",
//                  ESP_BD_ADDR_HEX(param->disconnect.remote_bda), param->disconnect.reason);
//         esp_ble_gap_start_advertising(&adv_params);
//         local_mtu = 23; // Reset MTU for a single connection
//         break;
//     case ESP_GATTS_CONF_EVT:
//         ESP_LOGI(GATTS_TAG, "确认接收, 状态 %d, 属性句柄 %d", param->conf.status, param->conf.handle);
//         if (param->conf.status != ESP_GATT_OK){
//             ESP_LOG_BUFFER_HEX(GATTS_TAG, param->conf.value, param->conf.len);
//         }
//         break;
//     case ESP_GATTS_OPEN_EVT:
//     case ESP_GATTS_CANCEL_OPEN_EVT:
//     case ESP_GATTS_CLOSE_EVT:
//     case ESP_GATTS_LISTEN_EVT:
//     case ESP_GATTS_CONGEST_EVT:
//     default:
//         break;
//     }
// }

// /**
//  * @brief GATT Server Profile B 事件处理函数
//  * 
//  * 处理蓝牙GATT服务器Profile B的各种事件，包括服务注册、读写操作、连接管理等
//  * 
//  * @param event GATT服务器事件类型
//  * @param gatts_if GATT服务器接口句柄
//  * @param param 指向GATT服务器回调参数结构体的指针
//  */
// static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
//     switch (event) {
//     case ESP_GATTS_REG_EVT:
//         ESP_LOGI(GATTS_TAG, "GATT服务注册, status %d, app_id %d, gatts_if %d", param->reg.status, param->reg.app_id, gatts_if);
//         gl_profile_tab[PROFILE_B_APP_ID].service_id.is_primary = true;
//         gl_profile_tab[PROFILE_B_APP_ID].service_id.id.inst_id = 0x00;
//         gl_profile_tab[PROFILE_B_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
//         gl_profile_tab[PROFILE_B_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_B;

//         esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_B_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_B);
//         break;
//     case ESP_GATTS_READ_EVT: {
//         ESP_LOGI(GATTS_TAG, "特征读取, conn_id %d, trans_id %" PRIu32 ", 句柄 %d", param->read.conn_id, param->read.trans_id, param->read.handle);
//         esp_gatt_rsp_t rsp;
//         memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
//         rsp.attr_value.handle = param->read.handle;
//         rsp.attr_value.len = 4;
//         rsp.attr_value.value[0] = 0xde;
//         rsp.attr_value.value[1] = 0xed;
//         rsp.attr_value.value[2] = 0xbe;
//         rsp.attr_value.value[3] = 0xef;
//         esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
//                                     ESP_GATT_OK, &rsp);
//         break;
//     }
//     case ESP_GATTS_WRITE_EVT: {
//         ESP_LOGI(GATTS_TAG, "特征写入, conn_id %d, trans_id %" PRIu32 ", 句柄 %d", param->write.conn_id, param->write.trans_id, param->write.handle);
//         if (!param->write.is_prep){
//             ESP_LOGI(GATTS_TAG, "值长度 %d, 值 ", param->write.len);
//             ESP_LOG_BUFFER_HEX(GATTS_TAG, param->write.value, param->write.len);
//             if (gl_profile_tab[PROFILE_B_APP_ID].descr_handle == param->write.handle && param->write.len == 2){
//                 uint16_t descr_value= param->write.value[1]<<8 | param->write.value[0];
//                 if (descr_value == 0x0001){
//                     if (b_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
//                         ESP_LOGI(GATTS_TAG, "通知已启用");
//                         uint8_t notify_data[15];
//                         for (int i = 0; i < sizeof(notify_data); ++i)
//                         {
//                             notify_data[i] = i%0xff;
//                         }
//                         //the size of notify_data[] need less than MTU size
//                         esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_B_APP_ID].char_handle,
//                                                 sizeof(notify_data), notify_data, false);
//                     }
//                 }else if (descr_value == 0x0002){
//                     if (b_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
//                         ESP_LOGI(GATTS_TAG, "指示已启用");
//                         uint8_t indicate_data[15];
//                         for (int i = 0; i < sizeof(indicate_data); ++i)
//                         {
//                             indicate_data[i] = i%0xff;
//                         }
//                         //the size of indicate_data[] need less than MTU size
//                         esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_B_APP_ID].char_handle,
//                                                 sizeof(indicate_data), indicate_data, true);
//                     }
//                 }
//                 else if (descr_value == 0x0000){
//                     ESP_LOGI(GATTS_TAG, "通知/指示已禁用");
//                 }else{
//                     ESP_LOGE(GATTS_TAG, "未知值");
//                 }

//             }
//         }
//         example_write_event_env(gatts_if, &b_prepare_write_env, param);
//         break;
//     }
//     case ESP_GATTS_EXEC_WRITE_EVT:
//         ESP_LOGI(GATTS_TAG,"执行写入事件");
//         esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
//         example_exec_write_event_env(&b_prepare_write_env, param);
//         break;
//     case ESP_GATTS_MTU_EVT:
//         ESP_LOGI(GATTS_TAG, "MTU交换, MTU %d", param->mtu.mtu);
//         break;
//     case ESP_GATTS_UNREG_EVT:
//         break;
//     case ESP_GATTS_CREATE_EVT:
//         ESP_LOGI(GATTS_TAG, "服务创建, 状态 %d,  服务句柄 %d", param->create.status, param->create.service_handle);
//         gl_profile_tab[PROFILE_B_APP_ID].service_handle = param->create.service_handle;
//         gl_profile_tab[PROFILE_B_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
//         gl_profile_tab[PROFILE_B_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_B;

//         esp_ble_gatts_start_service(gl_profile_tab[PROFILE_B_APP_ID].service_handle);
//         b_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
//         esp_err_t add_char_ret =esp_ble_gatts_add_char( gl_profile_tab[PROFILE_B_APP_ID].service_handle, &gl_profile_tab[PROFILE_B_APP_ID].char_uuid,
//                                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
//                                                         b_property,
//                                                         NULL, NULL);
//         if (add_char_ret){
//             ESP_LOGE(GATTS_TAG, "添加特征失败, 错误码 =%x",add_char_ret);
//         }
//         break;
//     case ESP_GATTS_ADD_INCL_SRVC_EVT:
//         break;
//     case ESP_GATTS_ADD_CHAR_EVT:
//         ESP_LOGI(GATTS_TAG, "特征添加, 状态 %d, 属性句柄 %d, 服务句柄 %d",
//                  param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

//         gl_profile_tab[PROFILE_B_APP_ID].char_handle = param->add_char.attr_handle;
//         gl_profile_tab[PROFILE_B_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
//         gl_profile_tab[PROFILE_B_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
//         esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_B_APP_ID].service_handle, &gl_profile_tab[PROFILE_B_APP_ID].descr_uuid,
//                                      ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
//                                      NULL, NULL);
//         break;
//     case ESP_GATTS_ADD_CHAR_DESCR_EVT:
//         gl_profile_tab[PROFILE_B_APP_ID].descr_handle = param->add_char_descr.attr_handle;
//         ESP_LOGI(GATTS_TAG, "描述符添加, 状态 %d, 属性句柄 %d, 服务句柄 %d",
//                  param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
//         break;
//     case ESP_GATTS_DELETE_EVT:
//         break;
//     case ESP_GATTS_START_EVT:
//         ESP_LOGI(GATTS_TAG, "服务启动, 状态 %d, 服务句柄 %d",
//                  param->start.status, param->start.service_handle);
//         break;
//     case ESP_GATTS_STOP_EVT:
//         break;
//     case ESP_GATTS_CONNECT_EVT:
//         ESP_LOGI(GATTS_TAG, "已连接, conn_id %d, 远程设备 "ESP_BD_ADDR_STR"",
//                  param->connect.conn_id, ESP_BD_ADDR_HEX(param->connect.remote_bda));
//         gl_profile_tab[PROFILE_B_APP_ID].conn_id = param->connect.conn_id;
//         break;
//     case ESP_GATTS_CONF_EVT:
//         ESP_LOGI(GATTS_TAG, "确认接收, 状态 %d, 属性句柄 %d", param->conf.status, param->conf.handle);
//         if (param->conf.status != ESP_GATT_OK){
//             ESP_LOG_BUFFER_HEX(GATTS_TAG, param->conf.value, param->conf.len);
//         }
//     break;
//     case ESP_GATTS_DISCONNECT_EVT:
//     case ESP_GATTS_OPEN_EVT:
//     case ESP_GATTS_CANCEL_OPEN_EVT:
//     case ESP_GATTS_CLOSE_EVT:
//     case ESP_GATTS_LISTEN_EVT:
//     case ESP_GATTS_CONGEST_EVT:
//     default:
//         break;
//     }
// }

// /**
//  * @brief GATT Server事件处理函数
//  * 
//  * 该函数负责处理GATT Server的各种事件，包括注册事件和其他GATT相关事件。
//  * 对于注册事件，会存储对应的gatts_if；对于其他事件，会分发给相应的profile回调函数处理。
//  * 
//  * @param event GATT Server事件类型
//  * @param gatts_if GATT Server接口标识符
//  * @param param 指向GATT Server回调参数结构体的指针
//  * @return 无返回值
//  */
// static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
// {
//     /* If event is register event, store the gatts_if for each profile */
//     if (event == ESP_GATTS_REG_EVT) {
//         if (param->reg.status == ESP_GATT_OK) {
//             gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
//         } else {
//             ESP_LOGI(GATTS_TAG, "注册应用失败, app_id %04x, 状态 %d",
//                     param->reg.app_id,
//                     param->reg.status);
//             return;
//         }
//     }

//     /* 遍历所有profile并调用相应的回调函数处理事件 */
//     do {
//         int idx;
//         for (idx = 0; idx < PROFILE_NUM; idx++) {
//             if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
//                     gatts_if == gl_profile_tab[idx].gatts_if) {
//                 if (gl_profile_tab[idx].gatts_cb) {
//                     gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
//                 }
//             }
//         }
//     } while (0);
// }


// /**
//  * @brief 通过BLE发送通知数据
//  * 
//  * 该函数用于向已连接的BLE客户端发送通知数据。函数会从全局配置表中获取
//  * 连接ID、特征句柄和GATT接口信息，然后调用ESP-IDF的GATT服务API发送通知。
//  * 
//  * @param data 指向要发送的数据缓冲区的指针
//  * @param len 要发送的数据长度（字节数）
//  * 
//  * @return 无返回值
//  */
// void ble_send_notify(const uint8_t *data, uint16_t len)
// {
//     // 从全局配置表中获取BLE连接相关信息
//     uint16_t conn_id   = gl_profile_tab[PROFILE_A_APP_ID].conn_id;
//     uint16_t char_hdl  = gl_profile_tab[PROFILE_A_APP_ID].char_handle;
//     esp_gatt_if_t g_if = gl_profile_tab[PROFILE_A_APP_ID].gatts_if;

//     // 检查设备是否已连接
//     if (conn_id == 0xFFFF) {               // 简单标记：未连接
//         ESP_LOGW(GATTS_TAG, "还没连，发不了");
//         return;
//     }
//     /* 如果 client 把 CCCD 写成 0x0001，notify 才允许发；这里偷个懒，直接发 */
//     esp_ble_gatts_send_indicate(g_if, conn_id, char_hdl,
//                                 len, data, false);   // false = notify
// }

// /**
//  * @brief 蓝牙通知发送任务函数
//  * 
//  * 该函数实现一个持续运行的任务，每2秒向蓝牙设备发送一次递增的消息通知。
//  * 消息格式为"Hello X"，其中X为从0开始递增的计数器值。
//  * 
//  * @param arg 任务参数指针（未使用）
//  * @return 无返回值
//  */
// static void notify_task(void *arg)
// {
//     uint8_t cnt = 0;
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(2000));
//         uint8_t buf[20];
//         int len = sprintf((char*)buf, "Hello %u", cnt++);
//         ble_send_notify(buf, len);
//     }
// }

// /**
//  * @brief 
//  *       初始化NVS、蓝牙控制器、Bluedroid栈，并注册GATT服务相关回调和应用
//  * 
//  * @param void 无参数
//  * @return void 无返回值
//  */
// void ble_task(void)
// {
//     esp_err_t ret;

//     // Initialize NVS.
//     ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK( ret );

//     #if CONFIG_EXAMPLE_CI_PIPELINE_ID
//     memcpy(test_device_name, esp_bluedroid_get_example_name(), ESP_BLE_ADV_NAME_LEN_MAX);
//     #endif

//     // 释放经典蓝牙内存以节省资源
//     ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

//     // 配置并初始化蓝牙控制器为BLE模式
//     esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
//     ret = esp_bt_controller_init(&bt_cfg);
//     if (ret) {
//         ESP_LOGE(GATTS_TAG, "%s 初始化控制器失败: %s", __func__, esp_err_to_name(ret));
//         return;
//     }

//     // 启用蓝牙控制器的BLE模式
//     ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
//     if (ret) {
//         ESP_LOGE(GATTS_TAG, "%s 启用控制器失败: %s", __func__, esp_err_to_name(ret));
//         return;
//     }

//     // 初始化并启用Bluedroid蓝牙协议栈
//     esp_bluedroid_config_t cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
//     ret = esp_bluedroid_init_with_cfg(&cfg);
//     if (ret) {
//         ESP_LOGE(GATTS_TAG, "%s 初始化蓝牙失败: %s", __func__, esp_err_to_name(ret));
//         return;
//     }
//     ret = esp_bluedroid_enable();
//     if (ret) {
//         ESP_LOGE(GATTS_TAG, "%s 启用蓝牙失败: %s", __func__, esp_err_to_name(ret));
//         return;
//     }
//     // 注册GATT服务事件回调函数
//     ret = esp_ble_gatts_register_callback(gatts_event_handler);
//     if (ret){
//         ESP_LOGE(GATTS_TAG, "gatts 注册错误, 错误码 = %x", ret);
//         return;
//     }
//     // 注册GAP事件回调函数
//     ret = esp_ble_gap_register_callback(gap_event_handler);
//     if (ret){
//         ESP_LOGE(GATTS_TAG, "gap 注册错误, 错误码 = %x", ret);
//         return;
//     }
//     // 注册两个GATT应用配置文件
//     ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID);
//     if (ret){
//         ESP_LOGE(GATTS_TAG, "gatts 应用注册错误, 错误码 = %x", ret);
//         return;
//     }
//     ret = esp_ble_gatts_app_register(PROFILE_B_APP_ID);
//     if (ret){
//         ESP_LOGE(GATTS_TAG, "gatts 应用注册错误, 错误码 = %x", ret);
//         return;
//     }
//     // 设置本地GATT MTU大小为500字节
//     esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
//     if (local_mtu_ret){
//         ESP_LOGE(GATTS_TAG, "设置本地 MTU 失败, 错误码 = %x", local_mtu_ret);
//     }
//     xTaskCreate(notify_task, "notify_task", 2048, NULL, 5, NULL);
//     return;
// }
