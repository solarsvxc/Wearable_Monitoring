#ifndef U8G2_PORT_STM32_H
#define U8G2_PORT_STM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "u8g2.h"
/*******************************************************************************
 * API
 ******************************************************************************/
/**
 * @brief 
 * 
 * @param u8x8 
 * @param msg 
 * @param arg_int 
 * @param arg_ptr 
 * @return uint8_t 
 */
uint8_t u8x8_stm32_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
/**
 * @brief 
 * 
 * @param u8x8 
 * @param msg 
 * @param arg_int 
 * @param arg_ptr 
 * @return uint8_t 
 */
uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);


#ifdef __cplusplus
}
#endif

#endif /* U8G2_PORT_STM32_H */
