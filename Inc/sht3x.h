#ifndef SHT3X_H
#define SHT3X_H

#include <stm32f411xe.h>
#include <stdint.h>

#define SHT3X_ADDR       (0x44)
#define SHT3X_SOFT_RST   (0x30A2)
#define SHT3X_STATUS_CLR (0x3041)
#define SHT3X_MEAS_HIGH  (0x24)


extern void i2c3_send_command(char slave_address,char cmd);
extern void i2c3_read_burst_byte(char slave_address,int n, char *data);
extern void delay_ms(uint32_t delay_in_ms);

typedef struct
{
    uint16_t temperature;
    uint16_t humidity;
} SHT3x_data_t;

/**
 * @brief module init 
 * 
 */
void SHT3X_init(void);

/**
 * @brief 
 * 
 */
void SHT3X_getData(SHT3x_data_t *raw_data);


#endif // SHT3X_H
