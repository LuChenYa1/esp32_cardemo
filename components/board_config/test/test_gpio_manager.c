/**
 * @file test_gpio_manager.c
 * @brief GPIO管理器单元测试
 */

#include "unity.h"
#include "gpio_manager.h"
#include "esp_log.h"

static const char *TAG = "GPIO_TEST";

/**
 * @brief 测试前的设置
 */
void setUp(void) {
    // 每个测试前重新初始化
    gpio_manager_init();
}

/**
 * @brief 测试后的清理
 */
void tearDown(void) {
    // 测试后清理（如果需要）
}

/**
 * @brief 测试GPIO管理器初始化
 */
TEST_CASE("GPIO管理器初始化", "[gpio_manager]") {
    esp_err_t ret = gpio_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

/**
 * @brief 测试GPIO注册成功
 */
TEST_CASE("GPIO注册成功", "[gpio_manager]") {
    gpio_manager_init();
    
    esp_err_t ret = gpio_manager_register(
        GPIO_NUM_12,
        GPIO_FUNC_GPIO_OUT,
        "test_module",
        "测试输出引脚"
    );
    
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

/**
 * @brief 测试GPIO冲突检测
 */
TEST_CASE("GPIO冲突检测", "[gpio_manager]") {
    gpio_manager_init();
    
    // 第一次注册应该成功
    esp_err_t ret1 = gpio_manager_register(
        GPIO_NUM_12,
        GPIO_FUNC_GPIO_OUT,
        "module_a",
        "第一次注册"
    );
    TEST_ASSERT_EQUAL(ESP_OK, ret1);
    
    // 第二次注册同一个GPIO应该失败
    esp_err_t ret2 = gpio_manager_register(
        GPIO_NUM_12,
        GPIO_FUNC_GPIO_IN,
        "module_b",
        "第二次注册（应该失败）"
    );
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret2);
}

/**
 * @brief 测试无效GPIO编号
 */
TEST_CASE("无效GPIO编号", "[gpio_manager]") {
    gpio_manager_init();
    
    // 测试负数GPIO编号
    esp_err_t ret1 = gpio_manager_register(
        -1,
        GPIO_FUNC_GPIO_OUT,
        "test_module",
        "无效GPIO"
    );
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret1);
    
    // 测试超出范围的GPIO编号
    esp_err_t ret2 = gpio_manager_register(
        100,
        GPIO_FUNC_GPIO_OUT,
        "test_module",
        "无效GPIO"
    );
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret2);
}

/**
 * @brief 测试GPIO占用检查
 */
TEST_CASE("GPIO占用检查", "[gpio_manager]") {
    gpio_manager_init();
    
    // 初始状态应该未占用
    TEST_ASSERT_FALSE(gpio_manager_is_allocated(GPIO_NUM_12, NULL));
    
    // 注册后应该已占用
    gpio_manager_register(
        GPIO_NUM_12,
        GPIO_FUNC_GPIO_OUT,
        "test_module",
        "测试引脚"
    );
    
    gpio_allocation_t allocation;
    TEST_ASSERT_TRUE(gpio_manager_is_allocated(GPIO_NUM_12, &allocation));
    TEST_ASSERT_EQUAL(GPIO_NUM_12, allocation.gpio_num);
    TEST_ASSERT_EQUAL(GPIO_FUNC_GPIO_OUT, allocation.function);
}

/**
 * @brief 测试多个GPIO注册
 */
TEST_CASE("多个GPIO注册", "[gpio_manager]") {
    gpio_manager_init();
    
    // 注册多个不同的GPIO
    TEST_ASSERT_EQUAL(ESP_OK, gpio_manager_register(GPIO_NUM_2, GPIO_FUNC_PWM, "motor", "电机1"));
    TEST_ASSERT_EQUAL(ESP_OK, gpio_manager_register(GPIO_NUM_3, GPIO_FUNC_PWM, "motor", "电机2"));
    TEST_ASSERT_EQUAL(ESP_OK, gpio_manager_register(GPIO_NUM_4, GPIO_FUNC_PWM, "motor", "电机3"));
    
    // 验证所有GPIO都已注册
    TEST_ASSERT_TRUE(gpio_manager_is_allocated(GPIO_NUM_2, NULL));
    TEST_ASSERT_TRUE(gpio_manager_is_allocated(GPIO_NUM_3, NULL));
    TEST_ASSERT_TRUE(gpio_manager_is_allocated(GPIO_NUM_4, NULL));
    
    // 验证未注册的GPIO
    TEST_ASSERT_FALSE(gpio_manager_is_allocated(GPIO_NUM_5, NULL));
}

/**
 * @brief 测试GPIO分配表验证
 */
TEST_CASE("GPIO分配表验证", "[gpio_manager]") {
    gpio_manager_init();
    
    // 注册一些GPIO
    gpio_manager_register(GPIO_NUM_12, GPIO_FUNC_GPIO_OUT, "test", "测试1");
    gpio_manager_register(GPIO_NUM_13, GPIO_FUNC_GPIO_IN, "test", "测试2");
    
    // 验证应该通过
    esp_err_t ret = gpio_manager_verify();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

/**
 * @brief 测试GPIO分配表打印（手动验证）
 */
TEST_CASE("GPIO分配表打印", "[gpio_manager][manual]") {
    gpio_manager_init();
    
    // 注册一些GPIO
    gpio_manager_register(GPIO_NUM_2, GPIO_FUNC_PWM, "motor", "电机1正向");
    gpio_manager_register(GPIO_NUM_3, GPIO_FUNC_PWM, "motor", "电机1反向");
    gpio_manager_register(GPIO_NUM_19, GPIO_FUNC_ADC, "gray_sensor", "左灰度传感器");
    gpio_manager_register(GPIO_NUM_20, GPIO_FUNC_ADC, "gray_sensor", "右灰度传感器");
    
    // 打印分配表（需要手动查看日志验证）
    ESP_LOGI(TAG, "以下是GPIO分配表（手动验证）：");
    gpio_manager_print_allocation_table();
}

/**
 * @brief 属性测试：GPIO引脚无冲突
 * 
 * 验证需求：12.9
 * 属性15：对于任意GPIO引脚，在系统初始化后，每个GPIO只能被分配给一个功能模块
 */
TEST_CASE("属性15：GPIO引脚无冲突", "[gpio_manager][property]") {
    gpio_manager_init();
    
    // 模拟注册所有系统GPIO
    const gpio_num_t test_gpios[] = {
        GPIO_NUM_2, GPIO_NUM_3, GPIO_NUM_4, GPIO_NUM_5,
        GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8, GPIO_NUM_9,
        GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_19, GPIO_NUM_20,
        GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38
    };
    
    const int num_gpios = sizeof(test_gpios) / sizeof(test_gpios[0]);
    
    // 第一轮：注册所有GPIO，应该全部成功
    for (int i = 0; i < num_gpios; i++) {
        esp_err_t ret = gpio_manager_register(
            test_gpios[i],
            GPIO_FUNC_GPIO_OUT,
            "test_module",
            "测试引脚"
        );
        TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "第一轮注册应该成功");
    }
    
    // 第二轮：尝试重复注册，应该全部失败
    for (int i = 0; i < num_gpios; i++) {
        esp_err_t ret = gpio_manager_register(
            test_gpios[i],
            GPIO_FUNC_GPIO_IN,
            "another_module",
            "重复注册"
        );
        TEST_ASSERT_EQUAL_MESSAGE(ESP_ERR_INVALID_STATE, ret, "重复注册应该失败");
    }
    
    // 验证：每个GPIO只被分配一次
    for (int i = 0; i < num_gpios; i++) {
        gpio_allocation_t allocation;
        bool allocated = gpio_manager_is_allocated(test_gpios[i], &allocation);
        TEST_ASSERT_TRUE(allocated);
        TEST_ASSERT_EQUAL_STRING("test_module", allocation.module_name);
    }
    
    ESP_LOGI(TAG, "属性15验证通过：所有%d个GPIO都只被分配一次", num_gpios);
}
