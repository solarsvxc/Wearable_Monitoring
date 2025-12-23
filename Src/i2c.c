/*    $$$$$$\   $$$$$$\  $$\         $$$$$$\  $$$$$$$\   $$$$$$\   $$\    $$\ $$\   $$\  $$$$$$\  
*    $$  __$$\ $$  __$$\ $$ |       $$  __$$\ $$  __$$\ $$  __$$\  $$ |   $$ |$$ |  $$ |$$  __$$\ 
*    $$ /  \__|$$ /  $$ |$$ |       $$ /  $$ |$$ |  $$ |$$ /  \__| $$ |   $$ |\$$\ $$  |$$ /  \__|
*    \$$$$$$\  $$ |  $$ |$$ |       $$$$$$$$ |$$$$$$$  |\$$$$$$\   \$$\  $$  | \$$$$  / $$ |      
*     \____$$\ $$ |  $$ |$$ |       $$  __$$ |$$  __$$<  \____$$\   \$$\$$  /  $$  $$<  $$ |      
*    $$\   $$ |$$ |  $$ |$$ |       $$ |  $$ |$$ |  $$ |$$\   $$ |   \$$$  /  $$  /\$$\ $$ |  $$\ 
*    \$$$$$$  | $$$$$$  |$$$$$$$$\  $$ |  $$ |$$ |  $$ |\$$$$$$  |    \$  /   $$ /  $$ |\$$$$$$  |
*     \______/  \______/ \________| \__|  \__|\__|  \__| \______/      \_/    \__|  \__| \______/ 
*/

#include "i2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define GPIOB_EN        (1U<<1)
#define GPIOA_EN        (1U<<0)

#define FLAG_BUSY       (1U<<1)
#define FLAG_ADDR       (1U<<1)
#define FLAG_STARTBIT   (1U<<0)
#define FLAG_TxE        (1U<<7)
#define FLAG_RxNE       (1U<<6)
#define FLAG_BTF        (1U<<2)

/*******************************************************************************
 * Code I2C1
 * PB6 - SCL
 * PB7 - SDA  
 * Standard mode
 * Master mode
 ******************************************************************************/
void i2c1_init(void)
{   
    /* PB6 to alternative function mode */
    GPIOB->MODER &=~ (1U<<12);
    GPIOB->MODER |=  (1U<<13);
    /* PB7 to alternative function mode */
    GPIOB->MODER &=~ (1U<<14);
    GPIOB->MODER |=  (1U<<15);

    /* PB6 - PB7 output type to open-drain */
    GPIOB->OTYPER |= (1U<<6);
    GPIOB->OTYPER |= (1U<<7);

    /* PB6 - PB7 port type to pull-up */
    GPIOB->PUPDR &=~ (1U<<13);
    GPIOB->PUPDR |=  (1U<<12);
    GPIOB->PUPDR &=~ (1U<<15);
    GPIOB->PUPDR |=  (1U<<14);

    /* PB6 as line SCL in alternative function /\-0100-/\ */
    GPIOB->AFR[0] &=~(1U<<27); 
    GPIOB->AFR[0] |= (1U<<26); 
    GPIOB->AFR[0] &=~(1U<<25); 
    GPIOB->AFR[0] &=~(1U<<24); 

    /* PB7 as line SDA in alternative function /\-0100-/\ */
    GPIOB->AFR[0] &=~(1U<<31); 
    GPIOB->AFR[0] |= (1U<<30); 
    GPIOB->AFR[0] &=~(1U<<29); 
    GPIOB->AFR[0] &=~(1U<<28); 
    
    /* I2C enable */
    RCC->APB1ENR |= (1U<<21);

    /* SWRTS[15] enter software reset i2c bus, reset state-machine */
    I2C1->CR1 |=  (1U<<15);
    /* SWRTS[15] get out reset */
    I2C1->CR1 &=~ (1U<<15);

    /* FREQ[5:0] set peripheral clock frequency EQUAL freq system, we's system running at 16MHz. /\== (16 = 00010000) ==/\ */
    I2C1->CR2 &= ~0x3F;
    I2C1->CR2 |= 16;

    /* CCR[11:0] Clock control register in Fm/Sm mode (Master mode), Standard mode */
    I2C1->CCR = 80;

    /* Standard mode max rise time */
    I2C1->TRISE = 17;

    /* Enable I2C module */
    I2C1->CR1 |=(1U<<0);
}

void i2c1_read_byte(char slave_address,char maddress, char *data)
{
    volatile int tmp;

    while (I2C1->SR2 & FLAG_BUSY){}

    I2C1->CR1 |= (1U<<8);

    while (!(I2C1->SR1 & FLAG_STARTBIT)){}
    /* Write */
    I2C1->DR = slave_address << 1;

    while (!(I2C1->SR1 & FLAG_ADDR)){}

    tmp = I2C1->SR2;

    I2C1->DR = maddress;

    while (!(I2C1->SR1 & FLAG_TxE)){}

    /* enable start communication */
    I2C1->CR1 |= (1U<<8);

    while (!(I2C1->SR1 & FLAG_STARTBIT)){}
    
    /* Read */
    I2C1->DR = slave_address << 1 | 1;

    while (!(I2C1->SR1 & FLAG_ADDR)){}

    I2C1->CR1 &=~(1U<<10);

    tmp = I2C1->SR2;

    I2C1->CR1 |= (1U<<9);

    while (!(I2C1->SR1 & FLAG_RxNE)){}
    
    *data++ = I2C1->DR;
}

void i2c1_read_burst_byte(char slave_address,char maddress, int n, char *data)
{
    volatile int tmp;
    
    /* wait busy is clear */
    while (I2C1->SR2 & FLAG_BUSY){}

    /* enable start communication */
    I2C1->CR1 |= (1U<<8);

    /* wait start flag is set*/
    while (!(I2C1->SR1 & FLAG_STARTBIT)){}

    /* Write */
    I2C1->DR = slave_address << 1;

    /* wait address sent */
    while (!(I2C1->SR1 & FLAG_ADDR)){}

    /* clear register sr2 */
    tmp = I2C1->SR2;

    /* write memory address to DataRegister */
    I2C1->DR = maddress;

    /* wait Transmit flag is set */
    while (!(I2C1->SR1 & FLAG_TxE)){}

    /* enable start communication  */
    I2C1->CR1 |= (1U<<8);

    /* wait start flag is set */
    while (!(I2C1->SR1 & FLAG_STARTBIT)){}
    
    /* Read */
    I2C1->DR = slave_address << 1 | 1;

    /* wait address sent */
    while (!(I2C1->SR1 & FLAG_ADDR)){}
    
    /* clear register sr2 */
    tmp = I2C1->SR2;

    /* enable bit ACK */
    I2C1->CR1 |= (1U<<10);

    while (n > 0U)
    {   
        /* if only data have one bit */
        if (n == 1U)
        {
            /* Disable bit acknowledge */
            I2C1->CR1 &=~(1U<<10);
            /* Generate stop */
            I2C1->CR1 |= (1U<<9);
            
            /* wait RXNE Flag is set */
            while (!(I2C1->SR1 & (FLAG_RxNE))){}
            /* get data from register */
            *data++ = I2C1->DR;
            break;
        } else
        {
            /* wait receiver flag is set */
            while (!(I2C1->SR1 & (FLAG_RxNE))){}
            /* read array data from DataRegister */
            *data++ = I2C1->DR;
            n--;
        }
    }
    
}

void i2c1_write_burst(char slave_address, char maddress,int n, char *data)
{
    volatile int tmp;
    
    /* wait busy flag is clear*/
    while (I2C1->SR2 & FLAG_BUSY){}

    /* enable start communication */
    I2C1->CR1 |= (1U<<8);
    
    /* wait start flag is set*/
    while (!(I2C1->SR1 & FLAG_STARTBIT)){}

    /* transmit slave address */
    I2C1->DR = slave_address << 1;

    /* wait address sent */
    while (!(I2C1->SR1 & FLAG_ADDR)){}

    /* clear address flag*/
    tmp = I2C1->SR2;

    /* wait flag transmit is set  */
    while (!(I2C1->SR1 & (FLAG_TxE))){}

    /* send memory address */
    I2C1->DR = maddress;
    
    for (int i = 0; i < n; i++)
    {
        while (!(I2C1->SR1 & (FLAG_TxE))){}
        I2C1->DR = *data++;
    }

    /* wait transmit all data success */
    while (!(I2C1->SR1 & (FLAG_BTF))){}

    /* stop communication */
    I2C1->CR1 |= (1U<<9);
}

void i2c1_u8g2_start(void)
{
    /* 1. Enable ACK */
    I2C1->CR1 |= (1U<<10);
    
    /* 2. Generate START */
    I2C1->CR1 |= (1U<<8);

    /* 3. Wait for SB flag */
    while (!(I2C1->SR1 & FLAG_STARTBIT));
}

void i2c1_u8g2_stop(void)
{
    /* Generate STOP */
    I2C1->CR1 |= (1U<<9);
}

void i2c1_u8g2_send_address(uint8_t address)
{
    /* Send Address (shifted by u8g2 already or manual?) 
     * u8g2 usually passes the 7-bit address, we need to handle R/W bit.
     * But usually u8g2 callback structure handles the full byte.
     * Let's assume 'address' is the full byte ready to go.
     */
    I2C1->DR = address;

    /* Wait for ADDR flag */
    while (!(I2C1->SR1 & FLAG_ADDR));

    /* Clear ADDR flag by reading SR1 then SR2 */
    volatile int tmp = I2C1->SR1;
    tmp = I2C1->SR2;
    (void)tmp; // prevent unused warning
}

void i2c1_u8g2_write_byte(uint8_t data)
{
    /* Wait for TxE (Transmit buffer empty) */
    while (!(I2C1->SR1 & FLAG_TxE));

    /* Write Data */
    I2C1->DR = data;

    /* Wait for BTF (Byte Transfer Finished) - Important for reliable chain */
    while (!(I2C1->SR1 & FLAG_BTF));
}
/*******************************************************************************
 * Code I2C2
 * PB10 - SCL
 * PB3 - SDA  
 * Standard mode
 * Master mode
 ******************************************************************************/
void i2c2_init(void)
{
    /* PB10 to alternative function mode */
    GPIOB->MODER &=~ (1U<<20);
    GPIOB->MODER |=  (1U<<21);
    /* PB3 to alternative function mode */
    GPIOB->MODER &=~ (1U<<6);
    GPIOB->MODER |=  (1U<<7);

    /* PB10 - PB3 output type to open-drain */
    GPIOB->OTYPER |= (1U<<10);
    GPIOB->OTYPER |= (1U<<3);

    /* PB10 - PB3 port type to pull-up */
    GPIOB->PUPDR &=~ (1U<<21);
    GPIOB->PUPDR |=  (1U<<20);
    GPIOB->PUPDR &=~ (1U<<7);
    GPIOB->PUPDR |=  (1U<<6);

    /* PB10 as line SCL in alternative function /\-0100-/\ */
    GPIOB->AFR[1] &=~(1U<<11); 
    GPIOB->AFR[1] |= (1U<<10); 
    GPIOB->AFR[1] &=~(1U<<9); 
    GPIOB->AFR[1] &=~(1U<<8); 

    /* PB3 as line SDA in alternative function /\-1001-/\ */
    GPIOB->AFR[0] |= (1U<<15); 
    GPIOB->AFR[0] &=~(1U<<14); 
    GPIOB->AFR[0] &=~(1U<<13); 
    GPIOB->AFR[0] |= (1U<<12); 
    
    /* I2C2 enable */
    RCC->APB1ENR |= (1U<<22);

    /* SWRTS[15] enter software reset i2c bus, reset state-machine */
    I2C2->CR1 |=  (1U<<15);
    /* SWRTS[15] get out reset */
    I2C2->CR1 &=~ (1U<<15);

    /* FREQ[5:0] set peripheral clock frequency EQUAL freq system, we's system running at 16MHz. /\== (16 = 00010000) ==/\ */
    I2C2->CR2 &= ~0x3F;
    I2C2->CR2 |= 16;

    /* CCR[11:0] Clock control register in Fm/Sm mode (Master mode), Standard mode */
    I2C2->CCR = 80;

    /* Standard mode max rise time */
    I2C2->TRISE = 17;

    /* Enable I2C module */
    I2C2->CR1 |=(1U<<0);
}

void i2c2_read_byte(char slave_address,char maddress, char *data)
{
volatile int tmp;

    while (I2C2->SR2 & FLAG_BUSY){}

    I2C2->CR1 |= (1U<<8);

    while (!(I2C2->SR1 & FLAG_STARTBIT)){}
    /* Write */
    I2C2->DR = slave_address << 1;

    while (!(I2C2->SR1 & FLAG_ADDR)){}

    tmp = I2C2->SR2;

    I2C2->DR = maddress;

    while (!(I2C2->SR1 & FLAG_TxE)){}

    /* enable start communication */
    I2C2->CR1 |= (1U<<8);

    while (!(I2C2->SR1 & FLAG_STARTBIT)){}
    
    /* Read */
    I2C2->DR = slave_address << 1 | 1;

    while (!(I2C2->SR1 & FLAG_ADDR)){}

    I2C2->CR1 &=~(1U<<10);

    tmp = I2C2->SR2;

    I2C2->CR1 |= (1U<<9);

    while (!(I2C2->SR1 & FLAG_RxNE)){}
    
    *data++ = I2C2->DR;
}

void i2c2_read_burst_byte(char slave_address,char maddress,int n, char *data)
{
    volatile int tmp;
    
    /* wait busy is clear */
    while (I2C2->SR2 & FLAG_BUSY){}

    /* enable start communication */
    I2C2->CR1 |= (1U<<8);

    /* wait start flag is set*/
    while (!(I2C2->SR1 & FLAG_STARTBIT)){}

    /* Write */
    I2C2->DR = slave_address << 1;

    /* wait address sent */
    while (!(I2C2->SR1 & FLAG_ADDR)){}

    /* clear register sr2 */
    tmp = I2C2->SR2;

    /* write memory address to DataRegister */
    I2C2->DR = maddress;

    /* wait Transmit flag is set */
    while (!(I2C2->SR1 & FLAG_TxE)){}

    /* enable start communication  */
    I2C2->CR1 |= (1U<<8);

    /* wait start flag is set */
    while (!(I2C2->SR1 & FLAG_STARTBIT)){}
    
    /* Read */
    I2C2->DR = slave_address << 1 | 1;

    /* wait address sent */
    while (!(I2C2->SR1 & FLAG_ADDR)){}
    
    /* clear register sr2 */
    tmp = I2C2->SR2;

    /* enable bit ACK */
    I2C2->CR1 |= (1U<<10);

    while (n > 0U)
    {   
        /* if only data have one bit */
        if (n == 1U)
        {
            /* Disable bit acknowledge */
            I2C2->CR1 &=~(1U<<10);
            /* Generate stop */
            I2C2->CR1 |= (1U<<9);
            
            /* wait RXNE Flag is set */
            while (!(I2C2->SR1 & (FLAG_RxNE))){}
            /* get data from register */
            *data++ = I2C2->DR;
            break;
        } else
        {
            /* wait receiver flag is set */
            while (!(I2C2->SR1 & (FLAG_RxNE))){}
            /* read array data from DataRegister */
            *data++ = I2C2->DR;
            n--;
        }
    }
    
}

void i2c2_write_burst(char slave_address, char maddress,int n, char *data)
{
    volatile int tmp;

    /* wait busy flag is clear*/
    while (I2C2->SR2 & FLAG_BUSY){}

    /* enable start communication */
    I2C2->CR1 |= (1U<<8);

    /* wait start flag is set*/
    while (!(I2C2->SR1 & FLAG_STARTBIT)){}

    /* transmit slave address */
    I2C2->DR = slave_address << 1;

    /* wait address sent */
    while (!(I2C2->SR1 & FLAG_ADDR)){}

    /* clear address flag*/
    tmp = I2C2->SR2;

    /* wait flag transmit is set  */
    while (!(I2C2->SR1 & (FLAG_TxE))){}

    /* send memory address */
    I2C2->DR = maddress;

    for (int i = 0; i < n; i++)
    {
        while (!(I2C2->SR1 & (FLAG_TxE))){}
        I2C2->DR = *data++;
    }

    /* wait transmit all data success */
    while (!(I2C2->SR1 & (FLAG_BTF))){}

    /* stop communication */
    I2C2->CR1 |= (1U<<9);
}

/*******************************************************************************
 * Code I2C3
 * PA8 - SCL
 * PB4 - SDA  
 * Standard mode
 * Master mode
 ******************************************************************************/
void i2c3_init(void)
{
    /* PA8 to alternative function mode */
    GPIOA->MODER &=~ (1U<<16);
    GPIOA->MODER |=  (1U<<17);
    /* PB4 to alternative function mode */
    GPIOB->MODER &=~ (1U<<8);
    GPIOB->MODER |=  (1U<<9);

    /* PA8 - PB4 output type to open-drain */
    GPIOA->OTYPER |= (1U<<8);
    GPIOB->OTYPER |= (1U<<4);

    /* PA8 - PB4 port type to pull-up */
    GPIOA->PUPDR &=~ (1U<<17);
    GPIOA->PUPDR |=  (1U<<16);
    GPIOB->PUPDR &=~ (1U<<9);
    GPIOB->PUPDR |=  (1U<<8);

    /* PA8 as line SCL in alternative function /\-0100-/\ */
    GPIOA->AFR[1] &=~(1U<<3); 
    GPIOA->AFR[1] |= (1U<<2); 
    GPIOA->AFR[1] &=~(1U<<1); 
    GPIOA->AFR[1] &=~(1U<<0); 

    /* PB4 as line SDA in alternative function /\-1001-/\ */
    GPIOB->AFR[0] |= (1U<<19); 
    GPIOB->AFR[0] &=~(1U<<18); 
    GPIOB->AFR[0] &=~(1U<<17); 
    GPIOB->AFR[0] |= (1U<<16); 
    
    /* I2C3 enable */
    RCC->APB1ENR |= (1U<<23);


    /* SWRTS[15] enter software reset i2c bus, reset state-machine */
    I2C3->CR1 |=  (1U<<15);
    /* SWRTS[15] get out reset */
    I2C3->CR1 &=~ (1U<<15);

    /* FREQ[5:0] set peripheral clock frequency EQUAL freq system, we's system running at 16MHz. /\== (16 = 00010000) ==/\ */
    I2C3->CR2 &= ~0x3F;
    I2C3->CR2 |= 16;

    /* CCR[11:0] Clock control register in Fm/Sm mode (Master mode), Standard mode */
    I2C3->CCR = 80;

    /* Standard mode max rise time */
    I2C3->TRISE = 17;

    /* Enable I2C module */
    I2C3->CR1 |=(1U<<0);
}

void i2c3_read_byte(char slave_address,char maddress, char *data)
{
    volatile int tmp;

    while (I2C3->SR2 & FLAG_BUSY){}

    I2C3->CR1 |= (1U<<8);

    while (!(I2C3->SR1 & FLAG_STARTBIT)){}
    /* Write */
    I2C3->DR = slave_address << 1;

    while (!(I2C3->SR1 & FLAG_ADDR)){}

    tmp = I2C3->SR2;

    I2C3->DR = maddress;

    while (!(I2C3->SR1 & FLAG_TxE)){}

    /* enable start communication */
    I2C3->CR1 |= (1U<<8);

    while (!(I2C3->SR1 & FLAG_STARTBIT)){}
    
    /* Read */
    I2C3->DR = slave_address << 1 | 1;

    while (!(I2C3->SR1 & FLAG_ADDR)){}

    I2C3->CR1 &=~(1U<<10);

    tmp = I2C3->SR2;

    I2C3->CR1 |= (1U<<9);

    while (!(I2C3->SR1 & FLAG_RxNE)){}
    
    *data++ = I2C3->DR;
}

void i2c3_read_burst_byte(char slave_address,int n, char *data)
{
    /* Wait busy is clear */
    while (I2C3->SR2 & FLAG_BUSY){}

    /* Enable ACK */
    I2C3->CR1 |= (1U<<10);

    /* Enable start communication */
    I2C3->CR1 |= (1U<<8);

    /* Wait start flag is set */
    while (!(I2C3->SR1 & FLAG_STARTBIT)){}
    
    /* Send Address + Read bit (LSB=1) */
    I2C3->DR = slave_address << 1 | 1;

    /* Wait address sent */
    while (!(I2C3->SR1 & FLAG_ADDR)){}
    
    /* Clear ADDR flag (Read SR1 then Read SR2) */
    volatile int tmp = I2C3->SR1;
    tmp = I2C3->SR2;
    (void)tmp;

    /* Loop reading data */
    while (n > 0U)
    {   
        /* if 1 byte */
        if (n == 1U)
        {
            /* Disable ACK */
            I2C3->CR1 &= ~(1U<<10);
            
            /* Send STOP */
            I2C3->CR1 |= (1U<<9);
            
            /* Wait data received (RXNE) */
            while (!(I2C3->SR1 & (FLAG_RxNE))){}
            
            /* Read Final Byte */
            *data++ = I2C3->DR;
            break;
        } 
        else
        {
            /* Wait data received */
            while (!(I2C3->SR1 & (FLAG_RxNE))){}
            
            /* Read Data */
            *data++ = I2C3->DR;
            n--;
        }
    }
}

void i2c3_send_command(char slave_address,char cmd)
{
    volatile int tmp;
    
    /* wait busy flag is clear*/
    while (I2C3->SR2 & FLAG_BUSY){}

    /* enable start communication */
    I2C3->CR1 |= (1U<<8);
    
    /* wait start flag is set*/
    while (!(I2C3->SR1 & FLAG_STARTBIT)){}

    /* transmit slave address */
    I2C3->DR = slave_address << 1;

    /* wait address sent */
    while (!(I2C3->SR1 & FLAG_ADDR)){}

    /* clear address flag*/
    tmp = I2C3->SR2;

    /* wait flag transmit is set  */
    while (!(I2C3->SR1 & (FLAG_TxE))){}

    /* send byte high (MSB) command */
    I2C3->DR = (cmd >> 8) & 0xFF;
    
    /* wait transmit all data success */
    while (!(I2C3->SR1 & (FLAG_BTF))){}
    
    /* wait flag transmit is set  */
    while (!(I2C3->SR1 & (FLAG_TxE))){}
    
    /* send byte low (LSB) command */
    I2C3->DR = (cmd & 0xFF);
    
    /* wait transmit all data success */
    while (!(I2C3->SR1 & (FLAG_BTF))){}

    /* stop communication */
    I2C3->CR1 |= (1U<<9);
}
