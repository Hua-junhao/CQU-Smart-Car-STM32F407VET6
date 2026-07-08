#include "line_sensor.h"

void LINE_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; // ????

    /* CH5, CH6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CH1, CH2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* CH3, CH4, CH7, CH8 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_3 | GPIO_Pin_2;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/* ????? 8 ????,?????(???)??? Bit ? 1 */
uint8_t LINE_ReadAll(void) {
    uint8_t state = 0;
    
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET) state |= (1 << 0); // CH1 (??)
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == Bit_RESET) state |= (1 << 1); // CH2
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET) state |= (1 << 2); // CH3
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == Bit_RESET) state |= (1 << 3); // CH4 (??)
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == Bit_RESET) state |= (1 << 4); // CH5 (??)
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == Bit_RESET) state |= (1 << 5); // CH6
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == Bit_RESET) state |= (1 << 6); // CH7
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == Bit_RESET) state |= (1 << 7); // CH8 (??)

    return state;
}