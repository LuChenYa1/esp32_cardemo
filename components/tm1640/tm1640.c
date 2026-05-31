/**
 * @file tm1640.c
 * @brief TM1640 LED点阵屏驱动实现
 * 
 * 功能：
 * - 8x16 LED点阵显示
 * - 软件模拟I2C协议
 * - 亮度调节（8级）
 * - 滚动动画支持
 * 
 * 接口：
 * - CLK: GPIO34 [SSA3]
 * - DIN: GPIO37 [SSA2]
 * 
 * 注意：
 * - 与TM1637复用相同引脚，不能同时使用
 */

#include "tm1640.h"
#include "gpio_manager.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TM1640";

/* 显示缓存：16列，每列8位对应8行 */
static uint8_t s_tm1640_gram[TM1640_COLS];
/* 显示控制寄存器：0x88 | 亮度(0-7) */
static uint8_t s_disp_ctrl = 0x8FU;

/**
 * @brief  延时函数（用于时序控制）
 * @param  无
 * @retval 无
 */
static void tm1640_delay_us(void)
{
    esp_rom_delay_us(10);  // 10微秒延时，满足TM1640时序要求
}

/**
 * @brief  获取 GSTEM 字模的某一列数据
 * @param  stream_col: 流式列索引（0-39）
 * @retval 该列的8位数据（bit0-bit6对应第0-6行，bit7空）
 */
static uint8_t tm1640_get_gstem_stream_col(uint8_t stream_col)
{
    // G字模（7列）
    static const uint8_t glyph_g[7] = {0x3EU, 0x20U, 0x20U, 0x2EU, 0x22U, 0x22U, 0x3EU};
    // S字模（7列）
    static const uint8_t glyph_s[7] = {0x3EU, 0x20U, 0x20U, 0x3EU, 0x02U, 0x02U, 0x3EU};
    // T字模（7列）
    static const uint8_t glyph_t[7] = {0x7FU, 0x08U, 0x08U, 0x08U, 0x08U, 0x08U, 0x08U};
    // E字模（7列）
    static const uint8_t glyph_e[7] = {0x7FU, 0x40U, 0x40U, 0x7EU, 0x40U, 0x40U, 0x7FU};
    // M字模（7列）
    static const uint8_t glyph_m[7] = {0x41U, 0x63U, 0x55U, 0x49U, 0x41U, 0x41U, 0x41U};
    
    const uint8_t *glyph_rows;
    uint8_t glyph_index;
    uint8_t col_in_glyph;
    uint8_t r;
    uint8_t bits = 0;
    
    // 每个字母占8列（7列字模+1列间隔）
    if ((stream_col % 8U) == 7U) {
        return 0x00U;  // 间隔列返回空白
    }
    
    glyph_index = (uint8_t)(stream_col / 8U);
    col_in_glyph = (uint8_t)(stream_col % 8U);
    
    // 选择对应的字母字模
    switch (glyph_index) {
        case 0: glyph_rows = glyph_g; break;
        case 1: glyph_rows = glyph_s; break;
        case 2: glyph_rows = glyph_t; break;
        case 3: glyph_rows = glyph_e; break;
        case 4: glyph_rows = glyph_m; break;
        default: return 0x00U;
    }
    
    // 将该列的7行数据打包成1字节
    for (r = 0; r < 7U; r++) {
        if (glyph_rows[r] & (uint8_t)(1U << (6U - col_in_glyph))) {
            bits |= (uint8_t)(1U << r);
        }
    }
    
    return bits;
}

/**
 * @brief  绘制全屏笑脸（用于字母滚动完成后的提示画面）
 * @param  无
 * @retval 无
 */
static void tm1640_draw_full_smile(void)
{
    uint8_t c;
    uint8_t r;
    
    tm1640_clear();
    
    // 绘制脸部轮廓（上下边）
    for (c = 2U; c <= 13U; c++) {
        tm1640_set_led(0U, c, 1U);  // 上边
        tm1640_set_led(7U, c, 1U);  // 下边
    }
    
    // 绘制脸部轮廓（左右边）
    for (r = 1U; r <= 6U; r++) {
        tm1640_set_led(r, 1U, 1U);   // 左边
        tm1640_set_led(r, 14U, 1U);  // 右边
    }
    
    // 绘制左眼（2x2）
    tm1640_set_led(2U, 5U, 1U);
    tm1640_set_led(2U, 6U, 1U);
    tm1640_set_led(3U, 5U, 1U);
    tm1640_set_led(3U, 6U, 1U);
    
    // 绘制右眼（2x2）
    tm1640_set_led(2U, 9U, 1U);
    tm1640_set_led(2U, 10U, 1U);
    tm1640_set_led(3U, 9U, 1U);
    tm1640_set_led(3U, 10U, 1U);
    
    // 绘制嘴巴（微笑弧线）
    for (c = 5U; c <= 10U; c++) {
        tm1640_set_led(5U, c, 1U);
    }
    tm1640_set_led(4U, 6U, 1U);
    tm1640_set_led(4U, 9U, 1U);
    
    tm1640_refresh();
}

/**
 * @brief  设置数据线电平
 * @param  state: 0=低电平, 1=高电平
 * @retval 无
 */
static void tm1640_set_din(uint8_t state)
{
    gpio_set_level(TM1640_DIN_GPIO, state);
}

/**
 * @brief  设置时钟线电平
 * @param  state: 0=低电平, 1=高电平
 * @retval 无
 */
static void tm1640_set_clk(uint8_t state)
{
    gpio_set_level(TM1640_CLK_GPIO, state);
}

/**
 * @brief  发送起始信号
 * @param  无
 * @retval 无
 */
static void tm1640_start(void)
{
    tm1640_set_din(1);
    tm1640_set_clk(1);
    tm1640_delay_us();
    tm1640_set_din(0);
    tm1640_delay_us();
    tm1640_set_clk(0);
    tm1640_delay_us();
}

/**
 * @brief  发送停止信号
 * @param  无
 * @retval 无
 */
static void tm1640_stop(void)
{
    tm1640_set_clk(0);
    tm1640_delay_us();
    tm1640_set_din(0);
    tm1640_delay_us();
    tm1640_set_clk(1);
    tm1640_delay_us();
    tm1640_set_din(1);
    tm1640_delay_us();
}

/**
 * @brief  写入一个字节数据
 * @param  data: 要写入的字节
 * @retval 无
 */
static void tm1640_write_byte(uint8_t data)
{
    uint8_t i;
    
    for (i = 0; i < 8; i++) {
        tm1640_set_clk(0);
        tm1640_delay_us();
        
        // LSB先发送
        if (data & 0x01U) {
            tm1640_set_din(1);
        } else {
            tm1640_set_din(0);
        }
        tm1640_delay_us();
        
        tm1640_set_clk(1);
        tm1640_delay_us();
        
        data >>= 1;
    }
    
    tm1640_set_clk(0);
    tm1640_delay_us();
}

/**
 * @brief  设置显示亮度
 * @param  level: 亮度等级（0-7，7最亮）
 * @retval 无
 */
void tm1640_set_brightness(uint8_t level)
{
    if (level > 7U) {
        level = 7U;
    }
    
    s_disp_ctrl = (uint8_t)(0x88U | level);
    
    tm1640_start();
    tm1640_write_byte(s_disp_ctrl);
    tm1640_stop();
}

/**
 * @brief  清空显示缓存（需调用 tm1640_refresh 才能生效）
 * @param  无
 * @retval 无
 */
void tm1640_clear(void)
{
    uint8_t i;
    for (i = 0; i < TM1640_COLS; i++) {
        s_tm1640_gram[i] = 0;
    }
}

/**
 * @brief  设置单个像素点
 * @param  row: 行号（0-7）
 * @param  col: 列号（0-15）
 * @param  on: 1=点亮, 0=熄灭
 * @retval 无
 */
void tm1640_set_led(uint8_t row, uint8_t col, uint8_t on)
{
    uint8_t mask;
    
    if (row >= TM1640_ROWS || col >= TM1640_COLS) {
        return;
    }
    
    mask = (uint8_t)(1U << row);
    
    if (on) {
        s_tm1640_gram[col] |= mask;
    } else {
        s_tm1640_gram[col] &= (uint8_t)(~mask);
    }
}

/**
 * @brief  刷新显示（将缓存数据写入 TM1640）
 * @param  无
 * @retval 无
 */
void tm1640_refresh(void)
{
    uint8_t i;
    
    // 设置数据命令（自动地址增加模式）
    tm1640_start();
    tm1640_write_byte(0x40U);
    tm1640_stop();
    
    // 设置起始地址并写入16列数据
    tm1640_start();
    tm1640_write_byte(0xC0U);
    for (i = 0; i < TM1640_COLS; i++) {
        tm1640_write_byte(s_tm1640_gram[i]);
    }
    tm1640_stop();
    
    // 设置显示控制（开显示+亮度）
    tm1640_start();
    tm1640_write_byte(s_disp_ctrl);
    tm1640_stop();
}

/**
 * @brief  初始化 TM1640（配置 GPIO 并清屏）
 * @param  无
 * @retval 无
 */
void tm1640_init(void)
{
    esp_err_t ret;
    
    // 注册GPIO到gpio_manager（检测冲突）
    ret = gpio_manager_register(TM1640_CLK_GPIO, GPIO_FUNC_GPIO_OUT, 
                                 "tm1640", "TM1640时钟线");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d注册失败，可能与其他模块冲突", TM1640_CLK_GPIO);
        return;
    }
    
    ret = gpio_manager_register(TM1640_DIN_GPIO, GPIO_FUNC_GPIO_OUT, 
                                 "tm1640", "TM1640数据线");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d注册失败，可能与其他模块冲突", TM1640_DIN_GPIO);
        return;
    }
    
    // 配置GPIO为输出模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TM1640_CLK_GPIO) | (1ULL << TM1640_DIN_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // 初始化引脚状态为高电平
    gpio_set_level(TM1640_CLK_GPIO, 1);
    gpio_set_level(TM1640_DIN_GPIO, 1);
    
    // 延时一段时间，让芯片稳定
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // 清屏并设置最高亮度
    tm1640_clear();
    tm1640_refresh();
    tm1640_set_brightness(7U);
    
    ESP_LOGI(TAG, "TM1640初始化成功 (CLK:GPIO%d, DIN:GPIO%d)", 
             TM1640_CLK_GPIO, TM1640_DIN_GPIO);
}

/**
 * @brief  显示 GSTEM 滚动动画（每次调用滚动一步）
 * @param  step_delay_ms: 每步延时（毫秒）
 * @retval 无
 * @note   需要在主循环中周期性调用此函数
 *         功能：1) 在16列窗口中左移滚动GSTEM
 *              2) 一轮滚动结束后切换到全屏笑脸
 *              3) 笑脸停留后清屏，再从右侧重新滚入G
 */
void tm1640_show_gstem_scroll_step(uint16_t step_delay_ms)
{
    static uint32_t last_tick = 0;
    static uint8_t stream_pos = 0;
    static uint8_t inited = 0;
    static uint8_t mode = 0;  // 0=滚动模式, 1=笑脸模式
    static uint32_t smile_start_tick = 0;
    
    uint8_t c;
    uint32_t now;
    const uint8_t total_cols = 40U;      // GSTEM总列数（5个字母×8列）
    const uint8_t trail_blank = 16U;     // 尾部空白列数
    const uint8_t wrap_cols = (uint8_t)(total_cols + trail_blank);
    const uint16_t smile_hold_ms = 1000U; // 笑脸停留时间
    
    // 获取系统时钟（毫秒）
    now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // 首次初始化
    if (!inited) {
        stream_pos = 0U;
        tm1640_clear();
        tm1640_refresh();
        inited = 1U;
        mode = 0U;
        last_tick = now;
        return;
    }
    
    // 笑脸模式：等待停留时间结束
    if (mode == 1U) {
        if ((now - smile_start_tick) < smile_hold_ms) {
            return;
        }
        // 笑脸停留结束，重新开始滚动
        stream_pos = 0U;
        tm1640_clear();
        tm1640_refresh();
        mode = 0U;
        last_tick = now;
        return;
    }
    
    // 滚动模式：检查是否到达步进时间
    if ((now - last_tick) < step_delay_ms) {
        return;
    }
    last_tick = now;
    
    // 左移一列
    for (c = 0; c < (TM1640_COLS - 1U); c++) {
        s_tm1640_gram[c] = s_tm1640_gram[c + 1U];
    }
    
    // 从右侧填入新列
    if (stream_pos < total_cols) {
        s_tm1640_gram[TM1640_COLS - 1U] = tm1640_get_gstem_stream_col(stream_pos);
    } else {
        s_tm1640_gram[TM1640_COLS - 1U] = 0x00U;  // 尾部空白
    }
    
    stream_pos++;
    
    // 一轮滚动完成，切换到笑脸模式
    if (stream_pos >= wrap_cols) {
        stream_pos = 0;
        mode = 1U;
        smile_start_tick = now;
        tm1640_draw_full_smile();
        return;
    }
    
    tm1640_refresh();
}
