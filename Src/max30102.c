/*    $$$$$$\   $$$$$$\  $$\         $$$$$$\  $$$$$$$\   $$$$$$\   $$\    $$\ $$\   $$\  $$$$$$\  
*    $$  __$$\ $$  __$$\ $$ |       $$  __$$\ $$  __$$\ $$  __$$\  $$ |   $$ |$$ |  $$ |$$  __$$\ 
*    $$ /  \__|$$ /  $$ |$$ |       $$ /  $$ |$$ |  $$ |$$ /  \__| $$ |   $$ |\$$\ $$  |$$ /  \__|
*    \$$$$$$\  $$ |  $$ |$$ |       $$$$$$$$ |$$$$$$$  |\$$$$$$\   \$$\  $$  | \$$$$  / $$ |      
*     \____$$\ $$ |  $$ |$$ |       $$  __$$ |$$  __$$<  \____$$\   \$$\$$  /  $$  $$<  $$ |      
*    $$\   $$ |$$ |  $$ |$$ |       $$ |  $$ |$$ |  $$ |$$\   $$ |   \$$$  /  $$  /\$$\ $$ |  $$\ 
*    \$$$$$$  | $$$$$$  |$$$$$$$$\  $$ |  $$ |$$ |  $$ |\$$$$$$  |    \$  /   $$ /  $$ |\$$$$$$  |
*     \______/  \______/ \________| \__|  \__|\__|  \__| \______/      \_/    \__|  \__| \______/ 
*/
#include "max30102.h"

int8_t MAX30102_read_id(void)
{
    uint8_t who_im_i = 0;
    
    i2c2_read_burst_byte(I2C_SL_ADDR,PART_ID,1, &who_im_i);
    
    if (who_im_i == PART_ID_VAL)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

void MAX30102_init(void)
{
    uint8_t config_buff;

    delay_ms(100);

    /* FIFO Config: smp_avg 1 | fifo roll 1 | FIFO almost full val 0 */
    config_buff = (0U << 5) | (1U << 4) | 0;
    i2c2_write_burst(I2C_SL_ADDR,FIFO_CONFIG,1,&config_buff);
    /* MODE Config: mode[2:0] = 0b11 (SpO2 mode, RED and IR)  */
    config_buff = (3U << 0);
    i2c2_write_burst(I2C_SL_ADDR,MODE_CONFIG,1,&config_buff);

    /* SPO2 Config:   
    *  SPO2_ADC_RGE[6:5] = 0b01
    *  SPO2_SR[4:2] = 0b001
    *  LED_PW[1:0] = 0b11 (18 - bit resolution)
    */
    config_buff = (1U << 5) | (1U << 2) | (3U<<0); 
    i2c2_write_burst(I2C_SL_ADDR,SPO2_CONFIG,1,&config_buff);

    /*LED Pulse Amplitude
    * LED1_PA[7:0] = 0x7Fh 
    * LED2_PA[7:0] = 0x7Fh
    */
    config_buff = LED_25_4_MA; /* 25.4mA  */
    i2c2_write_burst(I2C_SL_ADDR,LED1_PA_ADDR,1,&config_buff);
    i2c2_write_burst(I2C_SL_ADDR,LED2_PA_ADDR,1,&config_buff);

    /* Clear ptr */
    config_buff = 0x00;
    i2c2_write_burst(I2C_SL_ADDR, FIFO_WR_PTR, 1, &config_buff);
    i2c2_write_burst(I2C_SL_ADDR, OVF_COUNTER, 1, &config_buff);
    i2c2_write_burst(I2C_SL_ADDR, FIFO_RD_PTR, 1, &config_buff);
}

void MAX30102_getData(MAX30102_data_t *raw_data)
{
    uint8_t tx_buffer[6];

    i2c2_read_burst_byte(I2C_SL_ADDR,FIFO_DATA,6,&tx_buffer);
    
    raw_data->raw_bpm   = ((uint16_t)((tx_buffer[0] & 0x03) << 16) | (tx_buffer[1] << 8) | (tx_buffer[2]));
    raw_data->raw_spo2  = ((uint16_t)((tx_buffer[3] & 0x03) << 16) | (tx_buffer[4] << 8) | (tx_buffer[5]));
}

