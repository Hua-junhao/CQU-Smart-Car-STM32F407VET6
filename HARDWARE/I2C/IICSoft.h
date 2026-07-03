#ifndef _IICSOFT_H_
#define _IICSOFT_H_
#include "sys.h" 
#include "delay.h"

#define IIC_SCL_PIN 	GPIO_Pin_6
#define IIC_SDA_PIN 	GPIO_Pin_7
#define IIC_RCC				RCC_AHB1Periph_GPIOB
#define IIC_GPIO			GPIOB
   	   		   
//IO????
#define SDA_IN()  { IIC_GPIO->MODER &= ~( 3 << ( 7 * 2 ) ); IIC_GPIO->MODER |= 0 << ( 7 * 2 ); }	
#define SDA_OUT() { IIC_GPIO->MODER &= ~( 3 << ( 7 * 2 ) ); IIC_GPIO->MODER |= 1 << ( 7 * 2 ); } 

/*IO????*/
#define IIC_SCL    PBout(6) 
#define IIC_SDA    PBout(7)
#define READ_SDA   PBin(7)  

void IIC_Init( void ); 
void IIC_Send_Byte( uint8_t ucTxData );
uint8_t IIC_Read_Byte( uint8_t ucAck );
uint8_t IIC_Single_Write( uint8_t ucSlaveAddress, uint8_t ucREGAddress, uint8_t ucDdata );
uint8_t IIC_Single_Read( uint8_t ucSlaveAddress, uint8_t ucREGAddress );
uint8_t IIC_Write_Len( uint8_t ucSlaveAddress, uint8_t ucREGAddress, uint8_t ucLength, uint8_t *ucData );
uint8_t IIC_Read_Len( uint8_t ucSlaveAddress, uint8_t ucREGAddress, uint8_t ucLength, uint8_t *ucData );

#endif