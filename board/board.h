/**
 * @file board.h
 * @brief Board interface file
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date June 2019
 *
 * The purpose is to have functions related to the board e.g. reading pins, turning leds on, etc.
 *
 */
#ifndef BOARD_H_
#define BOARD_H_

	/* active low */
	#define SH_WAKE_GPIO_Pin  GPIO_PIN_1
	#define SH_WAKE_GPIO_Port GPIOB


/* Related to the SD Card */


    #define SD_ON_OFF_GPIO_Pin  GPIO_PIN_8
	#define SD_ON_OFF_GPIO_Port GPIOA

	#define SD_DET_GPIO_Pin  GPIO_PIN_9
	#define SD_DET_GPIO_Port GPIOA


/* LEDs primitives */
void Led_Toggle(unsigned int nled);
void Led_TurnOn(unsigned int nled);
void Led_TurnOff(unsigned int nled);

unsigned int STCC_FAULT_Read();
void FTDI_TurnOn();
void FTDI_TurnOff();

void AndroidLinkBoard_TurnOff();
void AndroidLinkBoard_TurnOn();

void AuxPow_TurnOff(void);
void AuxPow_TurnOn(void);

void SD_TurnOn(void);
void SD_TurnOff(void);

typedef enum {
	PWC_HW_DEF_DX2 = 0,
	PWC_HW_DEF_PG = 1,
	PWC_HW_DEF_LINX = 2,
	PWC_HW_UNDEF = 3,
} PWC_HW_TYPE;

PWC_HW_TYPE PWC_ReadType();


#endif /* BOARD_H_ */
