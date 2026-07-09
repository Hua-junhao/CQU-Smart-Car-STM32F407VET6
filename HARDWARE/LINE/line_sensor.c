#include "line_sensor.h"
#include "delay.h" // ?? ???? delay ?????????

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} LINE_GPIO_T;

static const LINE_GPIO_T g_line_gpio[LINE_SENSOR_COUNT] = {
    {LINE_CH1_PORT, LINE_CH1_PIN},
    {LINE_CH2_PORT, LINE_CH2_PIN},
    {LINE_CH3_PORT, LINE_CH3_PIN},
    {LINE_CH4_PORT, LINE_CH4_PIN},
    {LINE_CH5_PORT, LINE_CH5_PIN},
    {LINE_CH6_PORT, LINE_CH6_PIN},
    {LINE_CH7_PORT, LINE_CH7_PIN},
    {LINE_CH8_PORT, LINE_CH8_PIN}
};

void LINE_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;       
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;       

    GPIO_InitStructure.GPIO_Pin = LINE_CH5_PIN | LINE_CH6_PIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LINE_CH1_PIN | LINE_CH2_PIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LINE_CH3_PIN | LINE_CH4_PIN | LINE_CH7_PIN | LINE_CH8_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

uint8_t LINE_ReadChannel(uint8_t channel)
{
    const LINE_GPIO_T *line;

    if (channel == 0 || channel > LINE_SENSOR_COUNT) return 0;
    line = &g_line_gpio[channel - 1];

    /* ?? ????:3 ????????!
       ???? 3 ?????,???? 8 ???
       ??????? 2 ??????(1),???????????!
       ?? 100% ??????????????????????! */
    uint8_t sample_sum = 0;
    
    if (GPIO_ReadInputDataBit(line->port, line->pin) == Bit_SET) sample_sum++;
    delay_us(8);
    if (GPIO_ReadInputDataBit(line->port, line->pin) == Bit_SET) sample_sum++;
    delay_us(8);
    if (GPIO_ReadInputDataBit(line->port, line->pin) == Bit_SET) sample_sum++;

    if (sample_sum >= 2)
    {
        return 1; // ?????,????
    }
    return 0; // ??????,????
}

uint8_t LINE_ReadAll(void)
{
    uint8_t i;
    uint8_t state = 0;

    for (i = 0; i < LINE_SENSOR_COUNT; i++)
    {
        if (LINE_ReadChannel(i + 1))
        {
            state |= (uint8_t)(1U << (7 - i)); 
        }
    }
    return state;
}