/**
 * @file main_spiflash_test.c
 * @brief 外部 SPI Flash 读写测试入口
 *
 * 使用 components/spiflash 组件的封装 API，通过 ESP-IDF 分区接口
 * 完成对外部 SPI Flash 的：初始化 -> 擦除 -> 写入 -> 读回 -> 校验。
 *
 * 默认引脚（来自 spiflash.h，可按硬件实际接线在 cfg 中覆盖）：
 *   MOSI=GPIO11  MISO=GPIO13  SCLK=GPIO12  CS=GPIO10
 *
 * 启用方法：将 main/CMakeLists.txt 的 SRCS 切换到本文件后重新编译烧录。
 */

#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_partition.h"

#include "spiflash.h"

static const char *TAG = "SPIFLASH_TEST";

#define TEST_OFFSET   0
#define TEST_SIZE     256

static void dump_hex(const char *prefix, const uint8_t *buf, size_t len)
{
    char line[3 * 16 + 1];
    for (size_t i = 0; i < len; i += 16) {
        size_t n = (len - i) > 16 ? 16 : (len - i);
        for (size_t j = 0; j < n; j++) {
            snprintf(&line[j * 3], 4, "%02X ", buf[i + j]);
        }
        ESP_LOGI(TAG, "%s[%03u]: %s", prefix, (unsigned)i, line);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "===== 外部 SPI Flash 读写测试开始 =====");

    spiflash_config_t cfg = {
        .host_id     = SPIFLASH_DEFAULT_HOST_ID,
        .mosi_io_num = SPIFLASH_DEFAULT_MOSI_IO,
        .miso_io_num = SPIFLASH_DEFAULT_MISO_IO,
        .sclk_io_num = SPIFLASH_DEFAULT_SCLK_IO,
        .cs_io_num   = SPIFLASH_DEFAULT_CS_IO,
    };

    esp_err_t ret = spiflash_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spiflash_init 失败: %s", esp_err_to_name(ret));
        ESP_LOGE(TAG, "请检查：1)接线 2)3.3V 电源 3)CS/MOSI/MISO 是否接反 4)芯片型号是否支持");
        return;
    }

    const esp_partition_t *part = spiflash_get_partition();
    if (!part) {
        ESP_LOGE(TAG, "spiflash_get_partition 返回 NULL");
        return;
    }
    ESP_LOGI(TAG, "分区信息: label=%s, size=%" PRIu32 " KB, addr=0x%08" PRIx32 ", erase=%" PRIu32 " B",
             part->label, part->size / 1024, part->address, part->erase_size);

    uint8_t *wbuf = malloc(TEST_SIZE);
    uint8_t *rbuf = malloc(TEST_SIZE);
    if (!wbuf || !rbuf) {
        ESP_LOGE(TAG, "内存分配失败");
        free(wbuf);
        free(rbuf);
        spiflash_deinit();
        return;
    }

    for (int i = 0; i < TEST_SIZE; i++) {
        wbuf[i] = (uint8_t)(i ^ 0xA5);
    }
    memset(rbuf, 0, TEST_SIZE);

    ESP_LOGI(TAG, "执行 offset=%d, size=%d 的擦/写/读测试", TEST_OFFSET, TEST_SIZE);
    ret = spiflash_simple_read_write(TEST_OFFSET, TEST_SIZE, wbuf, rbuf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spiflash_simple_read_write 失败: %s", esp_err_to_name(ret));
        free(wbuf); free(rbuf);
        spiflash_deinit();
        return;
    }

    if (memcmp(wbuf, rbuf, TEST_SIZE) == 0) {
        ESP_LOGI(TAG, "数据一致，SPI Flash 读写正常");
    } else {
        ESP_LOGE(TAG, "数据不一致！写入与读回数据如下：");
        dump_hex("W", wbuf, 64);
        dump_hex("R", rbuf, 64);
        ESP_LOGE(TAG, "排查方向：MOSI/MISO 是否接反；频率过高（试试 spiflash.c 中 freq_mhz=10）；io_mode 是否匹配芯片");
    }

    free(wbuf);
    free(rbuf);

    ESP_LOGI(TAG, "===== 测试结束，保留 SPI Flash 已初始化状态 =====");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
