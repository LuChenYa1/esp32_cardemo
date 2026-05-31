/**
 * @file gpio_manager.c
 * @brief GPIO resource allocation and conflict detection.
 */

#include "gpio_manager.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "GPIO_MANAGER";

#define MAX_GPIO_NUM 49

static gpio_allocation_t gpio_allocation_table[MAX_GPIO_NUM];
static bool is_initialized = false;

static const char *get_function_name(gpio_function_t function)
{
    switch (function) {
        case GPIO_FUNC_UNUSED:      return "unused";
        case GPIO_FUNC_ADC:         return "adc";
        case GPIO_FUNC_PWM:         return "pwm";
        case GPIO_FUNC_UART_TX:     return "uart_tx";
        case GPIO_FUNC_UART_RX:     return "uart_rx";
        case GPIO_FUNC_I2C_SDA:     return "i2c_sda";
        case GPIO_FUNC_I2C_SCL:     return "i2c_scl";
        case GPIO_FUNC_GPIO_OUT:    return "gpio_out";
        case GPIO_FUNC_GPIO_IN:     return "gpio_in";
        case GPIO_FUNC_TOUCH:       return "touch";
        case GPIO_FUNC_DHT11:       return "dht11";
        case GPIO_FUNC_TM1637_CLK:  return "tm1637_clk";
        case GPIO_FUNC_TM1637_DIO:  return "tm1637_dio";
        default:                    return "unknown";
    }
}

esp_err_t gpio_manager_init(void)
{
    if (is_initialized) {
        ESP_LOGW(TAG, "GPIO manager already initialized");
        return ESP_OK;
    }

    for (int i = 0; i < MAX_GPIO_NUM; i++) {
        gpio_allocation_table[i].gpio_num = (gpio_num_t)i;
        gpio_allocation_table[i].function = GPIO_FUNC_UNUSED;
        gpio_allocation_table[i].module_name = NULL;
        gpio_allocation_table[i].description = NULL;
    }

    is_initialized = true;
    ESP_LOGI(TAG, "GPIO manager initialized");
    return ESP_OK;
}

static esp_err_t gpio_manager_ensure_initialized(void)
{
    if (is_initialized) {
        return ESP_OK;
    }

    return gpio_manager_init();
}

esp_err_t gpio_manager_register(gpio_num_t gpio_num,
                                gpio_function_t function,
                                const char *module_name,
                                const char *description)
{
    esp_err_t ret = gpio_manager_ensure_initialized();
    if (ret != ESP_OK) {
        return ret;
    }

    if (gpio_num < 0 || gpio_num >= MAX_GPIO_NUM) {
        ESP_LOGE(TAG, "Invalid GPIO number: %d", gpio_num);
        return ESP_ERR_INVALID_ARG;
    }

    if (gpio_allocation_table[gpio_num].function != GPIO_FUNC_UNUSED) {
        ESP_LOGE(TAG, "GPIO conflict detected:");
        ESP_LOGE(TAG, "  GPIO%d is already allocated", gpio_num);
        ESP_LOGE(TAG, "    existing module: %s", gpio_allocation_table[gpio_num].module_name);
        ESP_LOGE(TAG, "    existing function: %s", get_function_name(gpio_allocation_table[gpio_num].function));
        ESP_LOGE(TAG, "    existing description: %s", gpio_allocation_table[gpio_num].description);
        ESP_LOGE(TAG, "  requested allocation:");
        ESP_LOGE(TAG, "    new module: %s", module_name);
        ESP_LOGE(TAG, "    new function: %s", get_function_name(function));
        ESP_LOGE(TAG, "    new description: %s", description);
        return ESP_ERR_INVALID_STATE;
    }

    gpio_allocation_table[gpio_num].function = function;
    gpio_allocation_table[gpio_num].module_name = module_name;
    gpio_allocation_table[gpio_num].description = description;

    ESP_LOGI(TAG, "Registered GPIO%d: %s - %s (%s)",
             gpio_num, module_name, description, get_function_name(function));

    return ESP_OK;
}

bool gpio_manager_is_allocated(gpio_num_t gpio_num, gpio_allocation_t *allocation)
{
    if (gpio_manager_ensure_initialized() != ESP_OK) {
        return false;
    }

    if (gpio_num < 0 || gpio_num >= MAX_GPIO_NUM) {
        return false;
    }

    bool allocated = (gpio_allocation_table[gpio_num].function != GPIO_FUNC_UNUSED);
    if (allocated && allocation != NULL) {
        *allocation = gpio_allocation_table[gpio_num];
    }

    return allocated;
}

void gpio_manager_print_allocation_table(void)
{
    if (gpio_manager_ensure_initialized() != ESP_OK) {
        return;
    }

    ESP_LOGI(TAG, "========== GPIO allocation table ==========");
    ESP_LOGI(TAG, "GPIO | function      | module             | description");
    ESP_LOGI(TAG, "-----+---------------+--------------------+------------------------");

    int allocated_count = 0;
    for (int i = 0; i < MAX_GPIO_NUM; i++) {
        if (gpio_allocation_table[i].function != GPIO_FUNC_UNUSED) {
            ESP_LOGI(TAG, "%-4d | %-13s | %-18s | %s",
                     i,
                     get_function_name(gpio_allocation_table[i].function),
                     gpio_allocation_table[i].module_name,
                     gpio_allocation_table[i].description);
            allocated_count++;
        }
    }

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "Total allocated GPIOs: %d", allocated_count);
}

esp_err_t gpio_manager_verify(void)
{
    esp_err_t ret = gpio_manager_ensure_initialized();
    if (ret != ESP_OK) {
        return ret;
    }

    bool has_conflict = false;
    int allocated_count = 0;

    for (int i = 0; i < MAX_GPIO_NUM; i++) {
        if (gpio_allocation_table[i].function != GPIO_FUNC_UNUSED) {
            allocated_count++;

            if (gpio_allocation_table[i].module_name == NULL) {
                ESP_LOGE(TAG, "GPIO%d: module name is NULL", i);
                has_conflict = true;
            }

            if (gpio_allocation_table[i].description == NULL) {
                ESP_LOGE(TAG, "GPIO%d: description is NULL", i);
                has_conflict = true;
            }
        }
    }

    if (has_conflict) {
        ESP_LOGE(TAG, "GPIO allocation table verification failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "GPIO allocation table verified (%d allocated GPIOs)", allocated_count);
    return ESP_OK;
}
