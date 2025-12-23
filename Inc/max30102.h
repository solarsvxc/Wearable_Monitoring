#ifndef MAX30102_H_
#define MAX30102_H_

#include <stdint.h>

#define I2C_SL_ADDR       (0x57) /* Device i2c address */
#define PART_ID           (0xFF)
#define PART_ID_VAL       (0x15)

#define FIFO_CONFIG       (0x08)
#define FIFO_WR_PTR       (0x04)
#define FIFO_RD_PTR       (0x06)
#define FIFO_DATA         (0x07)

#define MODE_CONFIG       (0x09)
#define SPO2_CONFIG       (0x0A)

#define OVF_COUNTER       (0x05)

#define LED1_PA_ADDR      (0x0C) /* LED 1 Pulse Amplitude */
#define LED2_PA_ADDR      (0x0D) /* LED 2 Pulse Amplitude */

#define LED_0_MA          (0x00)
#define LED_0_2_MA        (0x01)
#define LED_0_4_MA        (0x02)
#define LED_3_MA          (0x0F)
#define LED_6_2_MA        (0x1F)
#define LED_12_6_MA       (0x3F)
#define LED_25_4_MA       (0x7F)
#define LED_51_MA         (0xFF)

extern void i2c2_read_burst_byte(char slave_address,char maddress,int n, char *data);
extern void i2c2_write_burst(char slave_address, char maddress,int n, char *data);
extern void delay_ms(uint32_t delay_ms);

typedef struct
{
    uint16_t raw_bpm;
    uint16_t raw_spo2;
} MAX30102_data_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief check id of device
 * 
 */
int8_t MAX30102_read_id(void);

/**
 * @brief init device
 * 
 */
void MAX30102_init(void);

/**
 * @brief read data from max30102 module
 * 
 */
void MAX30102_getData(MAX30102_data_t *raw_data);

#endif /* MAX30102_H_ */
