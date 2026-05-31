/**
 * @file spiflash.c
 * @brief SPI Flash存储模块实现
 * 
 * 功能：
 * - 外部SPI Flash初始化
 * - 数据读写操作
 * - 分区管理
 * 
 * 注意：
 * - 需要配置SPI总线和CS引脚
 * - 不要与内部Flash冲突
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>  // 添加inttypes.h以使用PRIu32等格式宏

#include "spiflash.h"
#include "esp_log.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/spi_pins.h"

static const char *TAG = "spiflash";

static esp_flash_t *s_flash_chip = NULL;
static const esp_partition_t *s_partition = NULL;
static bool s_initialized = false;

esp_err_t spiflash_init(const spiflash_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        return ESP_ERR_INVALID_ARG;
    }

    // 设置默认值
    spi_host_device_t host_id = config->host_id;
    int mosi_io_num = config->mosi_io_num;
    int miso_io_num = config->miso_io_num;
    int sclk_io_num = config->sclk_io_num;
    int cs_io_num = config->cs_io_num;

    if (host_id == 0) {
        host_id = SPIFLASH_DEFAULT_HOST_ID;
    }
    if (mosi_io_num < 0) {
        mosi_io_num = SPIFLASH_DEFAULT_MOSI_IO;
    }
    if (miso_io_num < 0) {
        miso_io_num = SPIFLASH_DEFAULT_MISO_IO;
    }
    if (sclk_io_num < 0) {
        sclk_io_num = SPIFLASH_DEFAULT_SCLK_IO;
    }
    if (cs_io_num < 0) {
        cs_io_num = SPIFLASH_DEFAULT_CS_IO;
    }

    // 配置SPI总线
    spi_bus_config_t bus_config = {
        .mosi_io_num = mosi_io_num,
        .miso_io_num = miso_io_num,
        .sclk_io_num = sclk_io_num,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    ESP_LOGI(TAG, "Initializing SPI bus with pins - MOSI: %d, MISO: %d, SCLK: %d, CS: %d", 
             mosi_io_num, miso_io_num, sclk_io_num, cs_io_num);
             
    esp_err_t ret = spi_bus_initialize(host_id, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // 配置Flash设备
    esp_flash_spi_device_config_t device_config = {
        .host_id = host_id,
        .cs_id = 0,
        .cs_io_num = cs_io_num,
        .io_mode = SPI_FLASH_DIO,
        .freq_mhz = 20,  // 降低频率以提高稳定性
    };

    ESP_LOGI(TAG, "Adding flash device to SPI bus");
    ret = spi_bus_add_flash_device(&s_flash_chip, &device_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add flash device: %s", esp_err_to_name(ret));
        spi_bus_free(host_id);
        return ret;
    }

    // 初始化Flash芯片
    ESP_LOGI(TAG, "Initializing flash chip");
    ret = esp_flash_init(s_flash_chip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize flash chip: %s", esp_err_to_name(ret));
        spi_bus_remove_flash_device(s_flash_chip);
        spi_bus_free(host_id);
        return ret;
    }

    // 获取Flash芯片信息
    uint32_t id = 0;
    uint32_t size = 0;
    esp_flash_read_id(s_flash_chip, &id);
    esp_flash_get_size(s_flash_chip, &size);
    
    ESP_LOGI(TAG, "Initialized external Flash, size=%" PRIu32 " KB, ID=0x%" PRIx32, size / 1024, id);

    // 注册分区
    const char *label = "spiflash_storage";
    ESP_LOGI(TAG, "Adding external Flash as a partition, label=\"%s\", size=%" PRIu32 " KB", label, size / 1024);
    
    ret = esp_partition_register_external(s_flash_chip, 0, size, label, 
                                         ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, &s_partition);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register partition: %s", esp_err_to_name(ret));
        return ret;
    }

    s_initialized = true;
    return ESP_OK;
}

esp_err_t spiflash_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    // 清理资源
    if (s_partition) {
        s_partition = NULL;
    }
    if (s_flash_chip) {
        spi_bus_remove_flash_device(s_flash_chip);
        s_flash_chip = NULL;
    }
    s_initialized = false;

    return ESP_OK;
}

const esp_partition_t* spiflash_get_partition(void)
{
    return s_partition;
}

esp_err_t spiflash_simple_read_write(size_t offset, size_t test_size, uint8_t *test_data, uint8_t *result_data)
{
    if (!s_partition || !test_data || !result_data) {
        return ESP_ERR_INVALID_ARG;
    }

    if (offset + test_size > s_partition->size) {
        ESP_LOGE(TAG, "Offset+Size (%zu+%zu) exceeds partition size (%"PRIu32")", 
                 offset, test_size, s_partition->size);
        return ESP_ERR_INVALID_ARG;
    }

    // 获取擦除扇区大小
    uint32_t erase_sector_size = s_partition->erase_size;
    ESP_LOGI(TAG, "Erase sector size: %"PRIu32" bytes", erase_sector_size);

    // 计算需要擦除的扇区范围
    size_t start_sector = offset / erase_sector_size;
    size_t end_sector = (offset + test_size - 1) / erase_sector_size;
    size_t erase_size = (end_sector - start_sector + 1) * erase_sector_size;
    size_t erase_offset = start_sector * erase_sector_size;

    ESP_LOGI(TAG, "Erasing sectors from offset %"PRIu32", total size: %zu", (uint32_t)erase_offset, erase_size);
    esp_err_t erase_ret = esp_partition_erase_range(s_partition, erase_offset, erase_size);
    if (erase_ret != ESP_OK) {
        ESP_LOGE(TAG, "Erase failed: %s", esp_err_to_name(erase_ret));
        return erase_ret;
    }

    // 执行写入操作
    ESP_LOGI(TAG, "Writing %zu bytes to offset %"PRIu32"", test_size, (uint32_t)offset);
    esp_err_t ret = esp_partition_write(s_partition, offset, test_data, test_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Write failed: %s", esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Write successful");
    }

    // 执行读取操作
    ESP_LOGI(TAG, "Reading %zu bytes from offset %"PRIu32"", test_size, (uint32_t)offset);
    ret = esp_partition_read(s_partition, offset, result_data, test_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read failed: %s", esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Read successful");
    }

    return ESP_OK;
}