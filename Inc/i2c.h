#ifndef I2C_H
#define I2C_H

#include <stm32f4xx.h>
#include <stdint.h>

/*******************************************************************************
 * API
 ******************************************************************************/

void i2c1_init(void);
/**
 * @brief  read 1 byte 1 from slave 
 * 
 * @param slave_address [in] i2c1 slave address
 * @param maddress      [in] memory address of slave
 * @param data          [in] 
 */
void i2c1_read_byte(char saddres,char maddress, char *data);
/**
 * @brief read array data from slave 
 * 
 * @param slave_address [in] i2c1 slave address
 * @param maddress      [in] memory address of slave
 * @param n             [in] size of array
 * @param data          [in]
 */
void i2c1_read_burst_byte(char slave_address,char maddress,int n, char *data);
/**
 * @brief write array data to slave
 * 
 * @param slave_address [in] i2c1 slave address
 * @param maddress      [in] memory address slave
 * @param n             [in] size of data
 * @param data          [in] 
 */
void i2c1_write_burst(char slave_address, char maddress,int n, char *data);

void i2c1_u8g2_start(void);
void i2c1_u8g2_stop(void);
void i2c1_u8g2_write_byte(uint8_t data);
void i2c1_u8g2_send_address(uint8_t address);

void i2c2_init(void);
/**
 * @brief  read 1 byte 1 from slave 
 * 
 * @param slave_address [in] i2c2 slave address
 * @param maddress      [in] memory address of slave
 * @param data          [in] 
 */
void i2c2_read_byte(char saddres,char maddress, char *data);
/**
 * @brief read array data from slave 
 * 
 * @param slave_address [in] i2c2 slave address
 * @param maddress      [in] memory address of slave
 * @param n             [in] size of array
 * @param data          [in]
 */
void i2c2_read_burst_byte(char slave_address,char maddress,int n, char *data);
/**
 * @brief write array data to slave
 * 
 * @param slave_address [in] i2c2 slave address
 * @param maddress      [in] memory address slave
 * @param n             [in] size of data
 * @param data          [in] 
 */
void i2c2_write_burst(char slave_address, char maddress,int n, char *data);

void i2c3_init(void);
/**
 * @brief  read 1 byte 1 from slave 
 * 
 * @param slave_address [in] i2c3 slave address
 * @param maddress      [in] memory address of slave
 * @param data          [in] 
 */
void i2c3_read_byte(char saddres,char maddress, char *data);
/**
 * @brief read array data from sht3x /\ Modified only for SHT3x /\
 * 
 * @param slave_address [in] i2c3 slave address
 * @param n             [in] size of array
 * @param data          [in]
 */
void i2c3_read_burst_byte(char slave_address,int n, char *data);
/**
 * @brief send command data to sht3x /\ Modified only for SHT3x /\
 * 
 * @param slave_address [in] i2c3 slave address
 * @param cmd           [in] command of SHT3x, MSB first -> LSB second 
 */
void i2c3_send_command(char slave_address,char cmd);

#endif // I2C_H
