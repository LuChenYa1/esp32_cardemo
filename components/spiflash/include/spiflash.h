#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#include "esp_err.h"
#include "driver/spi_master.h"  // 包含SPI相关的定义
#include "esp_partition.h"

#define SPIFLASH_DEFAULT_HOST_ID    SPI2_HOST
#define SPIFLASH_DEFAULT_MOSI_IO    11
#define SPIFLASH_DEFAULT_MISO_IO    13
#define SPIFLASH_DEFAULT_SCLK_IO    12
#define SPIFLASH_DEFAULT_CS_IO      10

/**
 * @brief SPI Flash初始化配置
 */
typedef struct {
    spi_host_device_t host_id;      /*!< SPI主机设备ID，默认SPI2_HOST */
    int mosi_io_num;                /*!< MOSI引脚号，默认GPIO11 */
    int miso_io_num;                /*!< MISO引脚号，默认GPIO13 */
    int sclk_io_num;                /*!< SCLK引脚号，默认GPIO12 */
    int cs_io_num;                  /*!< CS引脚号，默认GPIO10 */
} spiflash_config_t;

/**
 * @brief 初始化SPI Flash
 * 
 * @param config SPI Flash配置参数
 * @return esp_err_t
 */
esp_err_t spiflash_init(const spiflash_config_t *config);

/**
 * @brief 反初始化SPI Flash
 * 
 * @return esp_err_t
 */
esp_err_t spiflash_deinit(void);

/**
 * @brief 获取SPI Flash分区
 * 
 * @return const esp_partition_t* 返回分区指针，失败返回NULL
 */
const esp_partition_t* spiflash_get_partition(void);

/**
 * @brief 对SPI Flash进行简单的读写操作
 * 
 * @param offset 操作起始偏移地址
 * @param test_size 测试数据大小（字节）
 * @param test_data 要写入的数据
 * @param result_data 读取的结果数据
 * @return esp_err_t
 */
esp_err_t spiflash_simple_read_write(size_t offset, size_t test_size, uint8_t *test_data, uint8_t *result_data);

#endif /* __SPIFLASH_H__ */