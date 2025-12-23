#ifndef UART1_H_
#define UART1_H_

#include <stm32f411xe.h>
#include <stdint.h>

void uart1_init(void);
/**
 * @brief send each char per conversation
 * 
 * @param ch [in] character 
 */
void uart1_send_char(int ch);
/**
 * @brief Send array data 
 * 
 * @param data    [in] an array data like sensor data,...
 * @param length  [in] length of array data
 */
void uart1_send_array(uint16_t *data, uint16_t length);

/**
 * @brief Calculates the UART1 baudrate register value based on system frequency and desired baudrate
 * 
 * This function computes the appropriate baudrate configuration value for UART1 peripheral
 * based on the MCU's system clock frequency and the target communication baudrate.
 * 
 * @param frequency  [in]  System clock frequency in Hz (MCU operating frequency)
 * @param baudrate   [in]  Desired UART baudrate in bps (e.g., 9600, 115200)
 * 
 * @return uint16_t  [out] Baudrate divider/scaler value to be written to UART1 control register
 * 
 * @note The returned value should be configured in the UART1 baudrate register (BRR or similar)
 * @note Ensure frequency parameter matches the actual MCU clock configuration
 * 
 * @example
 * uint16_t brr_value = calculation_baudrate(72000000, 115200); // For 72MHz clock, 115200 baud
 */
static uint16_t calculation_baudrate( uint32_t frequency, uint32_t baudrate);

/**
 * @brief Set the baudrate object
 * 
 */
void set_baudrate(void);

#endif /* UART1_H_ */
