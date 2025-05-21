//=====[Libraries]=============================================================
//#include "mbed.h"
//#include "arm_book_lib.h"

#include "display.h"
#include "main.h"
#include "dwt.h"
#include <stdbool.h>

/********************** arm_book Defines *******************************/
//#include "arm_book_lib.h"
// Functional states
#ifndef OFF
#define OFF    0
#endif
#ifndef ON
#define ON     ( !OFF )
#endif

// Electrical states
#ifndef LOW
#define LOW    0
#endif
#ifndef HIGH
#define HIGH   ( !LOW )
#endif

//=====[Declaration of private defines]========================================
#define DISPLAY_IR_CLEAR_DISPLAY   0b00000001
#define DISPLAY_IR_ENTRY_MODE_SET  0b00000100
#define DISPLAY_IR_DISPLAY_CONTROL 0b00001000
#define DISPLAY_IR_FUNCTION_SET    0b00100000
#define DISPLAY_IR_SET_DDRAM_ADDR  0b10000000

#define DISPLAY_IR_ENTRY_MODE_SET_INCREMENT 0b00000010
#define DISPLAY_IR_ENTRY_MODE_SET_DECREMENT 0b00000000
#define DISPLAY_IR_ENTRY_MODE_SET_SHIFT     0b00000001
#define DISPLAY_IR_ENTRY_MODE_SET_NO_SHIFT  0b00000000

#define DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_ON  0b00000100
#define DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_OFF 0b00000000
#define DISPLAY_IR_DISPLAY_CONTROL_CURSOR_ON   0b00000010
#define DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF  0b00000000
#define DISPLAY_IR_DISPLAY_CONTROL_BLINK_ON    0b00000001
#define DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF   0b00000000

#define DISPLAY_IR_FUNCTION_SET_8BITS    0b00010000
#define DISPLAY_IR_FUNCTION_SET_4BITS    0b00000000
#define DISPLAY_IR_FUNCTION_SET_2LINES   0b00001000
#define DISPLAY_IR_FUNCTION_SET_1LINE    0b00000000
#define DISPLAY_IR_FUNCTION_SET_5x10DOTS 0b00000100
#define DISPLAY_IR_FUNCTION_SET_5x8DOTS  0b00000000

#define DISPLAY_20x4_LINE1_FIRST_CHARACTER_ADDRESS 0
#define DISPLAY_20x4_LINE2_FIRST_CHARACTER_ADDRESS 64
#define DISPLAY_20x4_LINE3_FIRST_CHARACTER_ADDRESS 20
#define DISPLAY_20x4_LINE4_FIRST_CHARACTER_ADDRESS 84

#define DISPLAY_RS_INSTRUCTION 0
#define DISPLAY_RS_DATA        1

#define DISPLAY_RW_WRITE 0
#define DISPLAY_RW_READ  1

#define DISPLAY_PIN_RS  4
#define DISPLAY_PIN_RW  5
#define DISPLAY_PIN_EN  6
#define DISPLAY_PIN_D0  7
#define DISPLAY_PIN_D1  8
#define DISPLAY_PIN_D2  9
#define DISPLAY_PIN_D3 10
#define DISPLAY_PIN_D4 11
#define DISPLAY_PIN_D5 12
#define DISPLAY_PIN_D6 13
#define DISPLAY_PIN_D7 14

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

#define OUT_D0 GPIO_PIN_3
#define OUT_PORT_D0 GPIOA

#define OUT_D1 GPIO_PIN_2
#define OUT_PORT_D1 GPIOA

#define OUT_D2 GPIO_PIN_10
#define OUT_PORT_D2 GPIOA

#define OUT_D3 GPIO_PIN_3
#define OUT_PORT_D3 GPIOB

#define OUT_D4 GPIO_PIN_5
#define OUT_PORT_D4 GPIOB

#define OUT_D5 GPIO_PIN_4
#define OUT_PORT_D5 GPIOB

#define OUT_D6 GPIO_PIN_10
#define OUT_PORT_D6 GPIOB

#define OUT_D7 GPIO_PIN_8
#define OUT_PORT_D7 GPIOA

#define OUT_D8 GPIO_PIN_9
#define OUT_PORT_D8 GPIOA

#define OUT_D9 GPIO_PIN_7
#define OUT_PORT_D9 GPIOC

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of private global variables]============
static display_t display;
static bool initial8BitCommunicationIsCompleted;

//=====[Declarations (prototypes) of private functions]========================
static void displayPinWrite( uint8_t pinName, int value );
static void displayDataBusWrite( uint8_t dataByte );
static void displayCodeWrite( bool type, uint8_t dataBus );

//=====[Implementations of public functions]===================================
void delay_us (uint32_t delay)
{
	uint32_t now,then;
	now = cycle_counter_time_us();
	then = now+delay;

	while(now <  then)
	{
		now=cycle_counter_time_us();
	}

}

void displayInit( displayConnection_t connection )
{
    display.connection = connection;

    initial8BitCommunicationIsCompleted = false;

    HAL_Delay(50);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_FUNCTION_SET |
                      DISPLAY_IR_FUNCTION_SET_8BITS );
    HAL_Delay( 5 );

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_FUNCTION_SET |
                      DISPLAY_IR_FUNCTION_SET_8BITS );
    HAL_Delay(1);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_FUNCTION_SET |
                      DISPLAY_IR_FUNCTION_SET_8BITS );


    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_FUNCTION_SET |
                              DISPLAY_IR_FUNCTION_SET_8BITS |
                              DISPLAY_IR_FUNCTION_SET_2LINES |
                              DISPLAY_IR_FUNCTION_SET_5x8DOTS );
            HAL_Delay(1);
        break;

        case DISPLAY_CONNECTION_GPIO_4BITS:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_FUNCTION_SET |
                              DISPLAY_IR_FUNCTION_SET_4BITS );
            HAL_Delay(1);

            initial8BitCommunicationIsCompleted = true;

            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_FUNCTION_SET |
                              DISPLAY_IR_FUNCTION_SET_4BITS |
                              DISPLAY_IR_FUNCTION_SET_2LINES |
                              DISPLAY_IR_FUNCTION_SET_5x8DOTS );
            HAL_Delay(1);
        break;
    }

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_DISPLAY_CONTROL |
                      DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_OFF |
                      DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF |
                      DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF );
    HAL_Delay(1);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_CLEAR_DISPLAY );
    HAL_Delay(1);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_ENTRY_MODE_SET |
                      DISPLAY_IR_ENTRY_MODE_SET_INCREMENT |
                      DISPLAY_IR_ENTRY_MODE_SET_NO_SHIFT );
    HAL_Delay(1);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_DISPLAY_CONTROL |
                      DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_ON |
                      DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF |
                      DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF );
    HAL_Delay(1);
}

void displayCharPositionWrite( uint8_t charPositionX, uint8_t charPositionY )
{
    switch( charPositionY ) {
        case 0:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE1_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay_us(37);
        break;

        case 1:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE2_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay_us(37);
        break;

        case 2:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE3_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay_us(37);
        break;

        case 3:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE4_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay_us(37);
        break;
    }
}

void displayStringWrite( const char * str )
{
    while (*str) {
        displayCodeWrite(DISPLAY_RS_DATA, *str++);
    }
}

//=====[Implementations of private functions]==================================
static void displayCodeWrite( bool type, uint8_t dataBus )
{
    if ( type == DISPLAY_RS_INSTRUCTION )
        displayPinWrite( DISPLAY_PIN_RS, DISPLAY_RS_INSTRUCTION );
	else
        displayPinWrite( DISPLAY_PIN_RS, DISPLAY_RS_DATA );
    displayPinWrite( DISPLAY_PIN_RW, DISPLAY_RW_WRITE );
    displayDataBusWrite( dataBus );
}

static void displayPinWrite( uint8_t pinName, int value )
{
    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS:
            switch( pinName ) {
            case DISPLAY_PIN_D0:
                            HAL_GPIO_WritePin( OUT_PORT_D0 ,OUT_D0, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_D1:
                            HAL_GPIO_WritePin( OUT_PORT_D1, OUT_D1, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_D2:
                            HAL_GPIO_WritePin(OUT_PORT_D2 ,OUT_D2, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_D3:
                            HAL_GPIO_WritePin(OUT_PORT_D3 ,OUT_D3, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_D4:
                            HAL_GPIO_WritePin(OUT_PORT_D4 ,OUT_D4, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_D5:
                            HAL_GPIO_WritePin(OUT_PORT_D5 ,OUT_D5, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_D6: //displayD6 = value;   break;
                            HAL_GPIO_WritePin(OUT_PORT_D6 ,OUT_D6, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_D7: //displayD7 = value;   break;
                            HAL_GPIO_WritePin(OUT_PORT_D7 ,OUT_D7, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_RS: //displayRs = value;   break;
                            HAL_GPIO_WritePin(OUT_PORT_D8 ,OUT_D8, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_EN: // displayEn = value;   break;
                            HAL_GPIO_WritePin(OUT_PORT_D9 ,OUT_D9, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                            case DISPLAY_PIN_RW: break;
                default: break;
            }
            break;

        case DISPLAY_CONNECTION_GPIO_4BITS:
            switch( pinName ) {
            case DISPLAY_PIN_D4:
                        HAL_GPIO_WritePin(OUT_PORT_D4 ,OUT_D4, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                        case DISPLAY_PIN_D5:
                        HAL_GPIO_WritePin(OUT_PORT_D5 ,OUT_D5, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                        case DISPLAY_PIN_D6: //displayD6 = value;   break;
                        HAL_GPIO_WritePin(OUT_PORT_D6 ,OUT_D6, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                        case DISPLAY_PIN_D7: //displayD7 = value;   break;
                         HAL_GPIO_WritePin(OUT_PORT_D7 ,OUT_D7, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                        case DISPLAY_PIN_RS: //displayRs = value;   break;
                        HAL_GPIO_WritePin(OUT_PORT_D8 ,OUT_D8, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                        case DISPLAY_PIN_EN: // displayEn = value;   break;
                        HAL_GPIO_WritePin(OUT_PORT_D9 ,OUT_D9, (value==0 ? GPIO_PIN_RESET  : GPIO_PIN_SET)); break;

                        case DISPLAY_PIN_RW: break;
                        default: break;
            }
            break;
    }
}

static void displayDataBusWrite( uint8_t dataBus )
{
    displayPinWrite( DISPLAY_PIN_EN, OFF );
    displayPinWrite( DISPLAY_PIN_D7, dataBus & 0b10000000 );
    displayPinWrite( DISPLAY_PIN_D6, dataBus & 0b01000000 );
    displayPinWrite( DISPLAY_PIN_D5, dataBus & 0b00100000 );
    displayPinWrite( DISPLAY_PIN_D4, dataBus & 0b00010000 );
    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS:
            displayPinWrite( DISPLAY_PIN_D3, dataBus & 0b00001000 );
            displayPinWrite( DISPLAY_PIN_D2, dataBus & 0b00000100 );
            displayPinWrite( DISPLAY_PIN_D1, dataBus & 0b00000010 );
            displayPinWrite( DISPLAY_PIN_D0, dataBus & 0b00000001 );
        break;

        case DISPLAY_CONNECTION_GPIO_4BITS:
            if ( initial8BitCommunicationIsCompleted == true) {
                displayPinWrite( DISPLAY_PIN_EN, ON );
                delay_us(37);
                displayPinWrite( DISPLAY_PIN_EN, OFF );
//                delay_us(37);
                displayPinWrite( DISPLAY_PIN_D7, dataBus & 0b00001000 );
                displayPinWrite( DISPLAY_PIN_D6, dataBus & 0b00000100 );
                displayPinWrite( DISPLAY_PIN_D5, dataBus & 0b00000010 );
                displayPinWrite( DISPLAY_PIN_D4, dataBus & 0b00000001 );
            }
        break;

    }
    displayPinWrite( DISPLAY_PIN_EN, ON );
    delay_us(37);
    displayPinWrite( DISPLAY_PIN_EN, OFF );
//    delay_us(37);
}
