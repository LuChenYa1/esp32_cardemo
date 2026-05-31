/**
 * @file us_delay.c
 * @brief 微秒级精确延时模块实现
 * 
 * 功能：
 * - 基于CPU时钟周期的精确延时
 * - 使用ESP32的ccount寄存器实现
 * - 适用于需要精确时序的场景（如DHT11、超声波等）
 * 
 * 注意：
 * - 延时期间会占用CPU，不适合长时间延时
 * - 延时精度受CPU频率影响
 */

#include <stdio.h>

/**
 * 内联函数，基于CPU时钟周期的精确延时
 * 使用ESP32的ccount寄存器（循环计数器）实现精确延时
 * @param ts: 延时的时钟周期数
 */
static __inline void delay_clock(int ts)
{
    uint32_t start, curr;
 
    // 读取当前CPU的循环计数器值作为起始时间
    __asm__ __volatile__("rsr %0, ccount" : "=r"(start));
    
    // 循环等待，直到经过的时钟周期数达到要求
    do
    {
        // 重复读取当前CPU的循环计数器值
        __asm__ __volatile__("rsr %0, ccount" : "=r"(curr));
        
    }while (curr - start <= ts);  // 当前时间减去起始时间小于等于目标周期数时继续循环
}
 
/**
 * 微秒级延时函数
 * @param us: 要延时的微秒数
 * 注意：假设CPU频率为160MHz，每个微秒需要160个时钟周期
 */
void udelay(int us)
{
    while (us--)  // 对于每一个微秒
    {
        delay_clock(160);  // 每微秒需要160个时钟周期（基于160MHz CPU频率）
    }
}
 
/**
 * 毫秒级延时函数
 * @param ms: 要延时的毫秒数
 * 注意：假设CPU频率为160MHz，每个毫秒需要160*1000个时钟周期
 */
void mdelay(int ms)
{
    while (ms--)  // 对于每一个毫秒
    {
        delay_clock(160*1000);  // 每毫秒需要160000个时钟周期（基于160MHz CPU频率）
    }
}