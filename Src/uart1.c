/*    $$$$$$\   $$$$$$\  $$\         $$$$$$\  $$$$$$$\   $$$$$$\   $$\    $$\ $$\   $$\  $$$$$$\  
*    $$  __$$\ $$  __$$\ $$ |       $$  __$$\ $$  __$$\ $$  __$$\  $$ |   $$ |$$ |  $$ |$$  __$$\ 
*    $$ /  \__|$$ /  $$ |$$ |       $$ /  $$ |$$ |  $$ |$$ /  \__| $$ |   $$ |\$$\ $$  |$$ /  \__|
*    \$$$$$$\  $$ |  $$ |$$ |       $$$$$$$$ |$$$$$$$  |\$$$$$$\   \$$\  $$  | \$$$$  / $$ |      
*     \____$$\ $$ |  $$ |$$ |       $$  __$$ |$$  __$$<  \____$$\   \$$\$$  /  $$  $$<  $$ |      
*    $$\   $$ |$$ |  $$ |$$ |       $$ |  $$ |$$ |  $$ |$$\   $$ |   \$$$  /  $$  /\$$\ $$ |  $$\ 
*    \$$$$$$  | $$$$$$  |$$$$$$$$\  $$ |  $$ |$$ |  $$ |\$$$$$$  |    \$  /   $$ /  $$ |\$$$$$$  |
*     \______/  \______/ \________| \__|  \__|\__|  \__| \______/      \_/    \__|  \__| \______/ 
*/

/* Specification 
* PA9  - UART1_TX
* PA10 - UART1_RX
*/

#include "uart1.h"

#define UART1_EN_CLK        (1U<<4)
#define USART1_EN           (1U<<13)
#define CR1_TE              (1U<<3)
#define SR_TxE              (1u<<7)

#define SYS_FREQ  16000000
#define ABP2_CLK  SYS_FREQ
#define TARGET_BAU 9600

static uint16_t calculation_baudrate( uint32_t frequency, uint32_t baudrate)
{
    return (uint16_t)((frequency + baudrate / 2) / baudrate );
}

void uart1_init(void)
{
    /* Clock Enable */
    RCC->APB2ENR  |= UART1_EN_CLK;

    /* PA9 to alternative function mode */
    GPIOA->MODER |= (1U<<19);
    GPIOA->MODER &=~(1U<<18);
    /* PA10 to alternative function mode */
    GPIOA->MODER |= (1U<<21);
    GPIOA->MODER &=~(1U<<20);

    /* PA9 to UART function is AF7 /\ 0111 /\ */
    GPIOA->AFR[1] &=~ (1U<<7); 
    GPIOA->AFR[1] |=  (1U<<6); 
    GPIOA->AFR[1] |=  (1U<<5); 
    GPIOA->AFR[1] |=  (1U<<4); 
    /* PA10 to UART function is AF7 /\ 0111 /\ */
    GPIOA->AFR[1] &=~ (1U<<11); 
    GPIOA->AFR[1] |=  (1U<<10); 
    GPIOA->AFR[1] |=  (1U<<9); 
    GPIOA->AFR[1] |=  (1U<<8); 
    
    /* Over sampling 16x */
    USART1->CR1  &=~(1U<<15); 
    /* Enable clock to APB2  */
    /* Baudrate 9600bps */
    set_baudrate();
    
    /* Transmitter enable */
    USART1->CR1 |= CR1_TE;
    /* UART1 enable */
    USART1->CR1 |= USART1_EN;
}

void set_baudrate(void)
{
    USART1->BRR = calculation_baudrate(SYS_FREQ, TARGET_BAU);
}

void uart1_send_char(int ch)
{
    while (!(USART1->SR & SR_TxE)){}
    USART1->DR = (ch & 0xFF);
}

void uart1_write_string(char *str) 
{
    while (*str)
    {
        uart1_send_char(*str);
        str++;
    }
}

void uart1_send_array(uint16_t *data, uint16_t length)
{
    for (int i = 0; i < length; i++)
    {
        uart1_send_char(data[i]);
    }
}
