/*    $$$$$$\   $$$$$$\  $$\         $$$$$$\  $$$$$$$\   $$$$$$\   $$\    $$\ $$\   $$\  $$$$$$\  
*    $$  __$$\ $$  __$$\ $$ |       $$  __$$\ $$  __$$\ $$  __$$\  $$ |   $$ |$$ |  $$ |$$  __$$\ 
*    $$ /  \__|$$ /  $$ |$$ |       $$ /  $$ |$$ |  $$ |$$ /  \__| $$ |   $$ |\$$\ $$  |$$ /  \__|
*    \$$$$$$\  $$ |  $$ |$$ |       $$$$$$$$ |$$$$$$$  |\$$$$$$\   \$$\  $$  | \$$$$  / $$ |      
*     \____$$\ $$ |  $$ |$$ |       $$  __$$ |$$  __$$<  \____$$\   \$$\$$  /  $$  $$<  $$ |      
*    $$\   $$ |$$ |  $$ |$$ |       $$ |  $$ |$$ |  $$ |$$\   $$ |   \$$$  /  $$  /\$$\ $$ |  $$\ 
*    \$$$$$$  | $$$$$$  |$$$$$$$$\  $$ |  $$ |$$ |  $$ |\$$$$$$  |    \$  /   $$ /  $$ |\$$$$$$  |
*     \______/  \______/ \________| \__|  \__|\__|  \__| \______/      \_/    \__|  \__| \______/ 
*/
#include "uart1.h"
#include "i2c.h"
#include "gpio_config.h"
#include "systick.h"
#include "max30102.h"
#include "sht3x.h"
#include "sh1106.h"
#include "u8g2_lib/clib/u8g2.h"
#include "u8g2_port_stm32.h"
#include "algorithm.h" 
#include <stdio.h>
#include "stm32f4xx.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFERSIZE 400

#pragma pack(push,1) /* Start Flag */
typedef struct 
{
    float bpm;
    float spo2;
    float temp;
    float hum;
    uint8_t spo2_valid;
    uint8_t bpm_valid;
} Converted_data_t;
#pragma pack(pop) /* End Flag */
/*******************************************************************************
 * Buffer
 ******************************************************************************/

static const uint8_t image_Layer_13_bits[] = {0x80,0x00,0x84,0x10,0x08,0x08,0xc0,0x01,0x31,0x46,0x12,0x24,0x08,0x08,0x08,0x08,0x08,0x08,0x12,0x24,0x31,0x46,0xc0,0x01,0x08,0x08,0x84,0x10,0x80,0x00,0x00,0x00};
static const uint8_t image_Layer_4_bits[] = {0x38,0x00,0x44,0x40,0xd4,0xa0,0x54,0x40,0xd4,0x1c,0x54,0x06,0xd4,0x02,0x54,0x02,0x54,0x06,0x92,0x1c,0x39,0x01,0x75,0x01,0x7d,0x01,0x39,0x01,0x82,0x00,0x7c,0x00};
static const uint8_t image_Layer_5_bits[] = {0x20,0x00,0x20,0x00,0x30,0x00,0x70,0x00,0x78,0x00,0xf8,0x00,0xfc,0x01,0xfc,0x01,0x7e,0x03,0xfe,0x02,0xff,0x06,0xff,0x07,0xfe,0x03,0xfe,0x03,0xfc,0x01,0xf0,0x00};

static uint32_t g_ir_buffer[BUFFERSIZE];
static uint32_t g_red_buffer[BUFFERSIZE];

static char g_scr_buffer[32];
/*******************************************************************************
 * Variables
 ******************************************************************************/

u8g2_t u8g2;
MAX30102_data_t   g_max_data;
SHT3x_data_t      g_sht3x_data;
Converted_data_t  g_conv_data;

/* Algorithm Output Variables */
float   g_calc_spo2;
int32_t g_calc_hr;
int8_t  g_spo2_valid_flag;
int8_t  g_hr_valid_flag;

static uint32_t g_sample_idx = 0;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void convert_data(void);
void display(void);
void uart_send_packet(Converted_data_t *data);

/*******************************************************************************
 * Main 
 ******************************************************************************/
int main(void)
{
    GPIOx_EN(GPIOA);
    GPIOx_EN(GPIOB);

    uart1_init();
    i2c1_init();
    i2c2_init();
    i2c3_init();

    MAX30102_init();
    SHT3X_init();

    u8g2_Setup_sh1106_128x64_noname_f(&u8g2,U8G2_R0,u8x8_byte_stm32_hw_i2c,u8x8_stm32_gpio_and_delay);
    u8g2_SetI2CAddress(&u8g2,(SH1106_I2C_ADDR << 1));
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2,0);

    while (1)
    {
        convert_data();
        if (g_sample_idx % 50 == 0) 
        {
            display();
            u8g2_SendBuffer(&u8g2); 
        }
    }
}

/**
 * @brief Converts raw sensor data from SHT3x and MAX30102 sensors into processed values.
 * 
 * This function performs the following operations:
 * - Every 100 samples: Reads temperature and humidity from SHT3x sensor and converts
 *   raw 16-bit values to actual temperature (°C) and humidity (%) using calibration formulas.
 * - Continuously: Reads raw PPG (photoplethysmography) data from MAX30102 sensor.
 * - Buffers: Stores raw red and IR LED values into circular buffers until BUFFERSIZE is reached.
 * - Processing: Once buffer is full, calls the Maxim algorithm to calculate heart rate (BPM)
 *   and blood oxygen saturation (SpO2) with validity flags.
 * - Reset: Clears the sample index to restart buffering.
 * 
 * @note Requires global variables: g_sample_idx, g_sht3x_data, g_max_data,
 *       g_conv_data, g_red_buffer, g_ir_buffer, g_calc_spo2, g_calc_hr, etc.
 * 
 * @param None
 * @return None
 * 
 * @see SHT3X_getData()
 * @see MAX30102_getData()
 * @see maxim_heart_rate_and_oxygen_saturation()
 */
void convert_data(void) 
{
    if(g_sample_idx % 100 == 0)
    {
        /* SHT3x processing raw data */
        SHT3X_getData(&g_sht3x_data);
        g_conv_data.temp = -45.0f + (175.0f * (((float)(g_sht3x_data.temperature)) / (65536.0f - 1.0f)));
        g_conv_data.hum  = (100.0f * (((float)(g_sht3x_data.humidity))  / (65536.0f - 1.0f)));
    }

    /* MAX30102 processing raw data */
    MAX30102_getData(&g_max_data);
    
    if (g_sample_idx < BUFFERSIZE)
    {

        g_red_buffer[g_sample_idx] = g_max_data.raw_bpm;
        g_ir_buffer[g_sample_idx]  = g_max_data.raw_spo2;
        g_sample_idx++;
    } else
    {
        /* DO NOTHING */
    }

    if (g_sample_idx == BUFFERSIZE)
    {
        maxim_heart_rate_and_oxygen_saturation(
            g_ir_buffer, 
            BUFFERSIZE, 
            g_red_buffer, 
            &g_calc_spo2, 
            &g_spo2_valid_flag, 
            &g_calc_hr, 
            &g_hr_valid_flag
        );
        /* Store results in global variable */
        g_conv_data.spo2        = g_calc_spo2;
        g_conv_data.bpm         = (float)g_calc_hr;
        g_conv_data.spo2_valid  = g_spo2_valid_flag;
        g_conv_data.bpm_valid   = g_hr_valid_flag;
        
        /* Send struct to esp32 */
        uart_send_packet(&g_conv_data);

        g_sample_idx = 0;
    } else
    {
        /* DO NOTHING */
    }
} 

/**
 * @brief Displays wearable health monitoring data on a 128x64 OLED screen
 * 
 * This function renders a complete user interface showing real-time vital signs
 * including temperature, blood oxygen saturation (SpO2), and heart rate (BPM).
 * 
 * The display layout consists of:
 * - Header section with title "Wearable Monitoring"
 * - Left panel: Temperature and SpO2 readings with icons
 * - Right panel: BPM (Heart Rate) reading with icon
 * - Decorative frames and divider lines for visual separation
 * 
 * @details
 * The function performs the following operations:
 * - Clears the display buffer
 * - Sets bitmap and font modes for rendering
 * - Draws border frame and divider lines
 * - Displays title text in Helvetica Bold 8pt font
 * - Renders two bitmap icons (medical/sensor icons)
 * - Formats and displays temperature value in Celsius (1 decimal place)
 * - Formats and displays SpO2 percentage value
 * - Formats and displays BPM value (1 decimal place)
 * - Adds labels and units (°C, %) for clarity
 * 
 * @note
 * - Requires g_conv_data structure containing: temp, spo2, bpm fields
 * - Requires g_scr_buffer for temporary string formatting
 * - Requires u8g2 display driver initialized
 * - Requires bitmap resources: image_Layer_4_bits, image_Layer_5_bits, image_Layer_13_bits
 * 
 * @return void
 */
void display(void) 
{
    
    u8g2_ClearBuffer(&u8g2); 
    u8g2_SetBitmapMode(&u8g2, 1);
    u8g2_SetFontMode(&u8g2, 1);

    // Layer 1
    u8g2_DrawFrame(&u8g2, 0, 0, 128, 64);

    // Layer 2
    u8g2_DrawLine(&u8g2, 0, 14, 127, 14);

    // Layer 3
    u8g2_SetFont(&u8g2, u8g2_font_helvB08_tr);
    u8g2_DrawStr(&u8g2, 12, 11, "Wearable Monitoring");

    // Layer 4
    u8g2_DrawXBM(&u8g2, 4, 19, 16, 16, image_Layer_4_bits);

    // Layer 5
    u8g2_DrawXBM(&u8g2, 6, 42, 11, 16, image_Layer_5_bits);

    // Layer 6
    u8g2_DrawLine(&u8g2, 89, 14, 89, 63);

    // Layer 7
    u8g2_SetFont(&u8g2, u8g2_font_haxrcorp4089_tr);
    sprintf(g_scr_buffer,"%.1f",g_conv_data.temp);
    u8g2_DrawStr(&u8g2, 31, 36, g_scr_buffer);

    // Layer 8
    u8g2_SetFont(&u8g2, u8g2_font_helvB08_tr);
    u8g2_DrawStr(&u8g2, 23, 26, "Temperature");

    // Layer 9
    u8g2_DrawStr(&u8g2, 23, 50, "SpO2");

    // Layer 10
    u8g2_SetFont(&u8g2, u8g2_font_haxrcorp4089_tr);
    sprintf(g_scr_buffer,"%.1f",g_conv_data.spo2);
    u8g2_DrawStr(&u8g2, 31, 60, g_scr_buffer);

    // Layer 11
    u8g2_SetFont(&u8g2, u8g2_font_helvB08_tr);
    u8g2_DrawStr(&u8g2, 97, 50, "BPM");

    // Layer 12
    u8g2_SetFont(&u8g2, u8g2_font_haxrcorp4089_tr);
    sprintf(g_scr_buffer,"%.1f",g_conv_data.bpm);
    u8g2_DrawStr(&u8g2, 99, 59, g_scr_buffer);

    // Layer 13
    u8g2_DrawXBM(&u8g2, 100, 19, 15, 16, image_Layer_13_bits);

    // Layer 14
    u8g2_SetFont(&u8g2, u8g2_font_helvB08_tr);
    u8g2_DrawStr(&u8g2, 58, 60, "%");

    // Layer 15
    u8g2_DrawUTF8(&u8g2, 58, 36, "°C");
}

/**
 * @brief Sends a data packet over UART with header, payload, and checksum verification
 * 
 * This function transmits a Converted_data_t structure over UART with the following format:
 * - Header: 0xAA, 0x55 (2 bytes)
 * - Payload: all bytes of the Converted_data_t structure
 * - Checksum: sum of all payload bytes (1 byte)
 * 
 * @param data Pointer to the Converted_data_t structure to be transmitted
 * 
 * @note The checksum is calculated as a simple sum of all data bytes.
 *       The receiver can verify data integrity by recalculating this checksum.
 * 
 * @warning Checksum calculation does not handle overflow/wraparound explicitly.
 *          It relies on uint8_t automatic overflow behavior.
 */
void uart_send_packet(Converted_data_t *data)
{
    uint8_t *ptr = (uint8_t *)data; 
    uint8_t checksum = 0;
    
    uart1_send_char(0xAA);
    uart1_send_char(0x55);
    
    for (int i = 0; i < sizeof(Converted_data_t); i++)
    {
        uart1_send_char(ptr[i]); 
        checksum += ptr[i];      
    }
    uart1_send_char(checksum);
}
