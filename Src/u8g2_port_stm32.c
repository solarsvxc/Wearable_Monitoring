#include "systick.h"
#include "u8g2_port_stm32.h"

extern void i2c1_u8g2_start(void);
extern void i2c1_u8g2_stop(void);
extern void i2c1_u8g2_write_byte(uint8_t data);
extern void i2c1_u8g2_send_address(uint8_t address);

uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
        case U8X8_MSG_BYTE_SEND:
        {
            uint8_t *data = (uint8_t *)arg_ptr;
            while(arg_int > 0)
            {
                i2c1_u8g2_write_byte(*data);
                data++;
                arg_int--;
            }
            break;
        }
        case U8X8_MSG_BYTE_START_TRANSFER:
        {
            i2c1_u8g2_start();
            i2c1_u8g2_send_address(u8x8_GetI2CAddress(u8x8));
            break;
        }
        case U8X8_MSG_BYTE_END_TRANSFER:
            i2c1_u8g2_stop();
            break;
        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_stm32_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
        case U8X8_MSG_DELAY_MILLI:
            delay_ms(arg_int);
            break;
        case U8X8_MSG_DELAY_10MICRO:
            delay_us(10);
            break;
        case U8X8_MSG_DELAY_100NANO:
            delay_us(1);
            break;
        default:
            return 0;
    }
    return 1;
}