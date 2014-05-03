//******************************************************************************
//              ОСНОВНОЙ ЗАГОЛОВОЧНЫЙ ФАЙЛ ПРОЕКТА
//******************************************************************************

//**************Подключаемые файлы**********************************************
#include "stm32f10x.h"
#include "eeprom.h"
//**************Макроопределения************************************************
#define BufferSize 512       //Размер буферов    байт

#define ERROR 0
#define ERROR_SATUS 1
#define SL_ST1 2
#define SL_ST2 3
#define SETTINGS 4
#define Q1 5
#define Q2 6
#define SETTINGS_2 7
#define TERM 8
#define GIST 9
#define PHONE1 52
#define PHONE2 58
//**************Определение собственных типов***********************************

typedef enum //Определение режима работы светодиода
{
OFF = 0,
RED,
GREEN,
ORANGE,
BLINK_RED,
BLINK_GREN,
BLINK_RED_GREEN
}LedMode_TypeDef;

//**************Глобальные переменные доступные из других файлов****************
extern LedMode_TypeDef Led1_state, Led2_state; //Состояние светодиодов
extern uint32_t delay_Counter; //Интервал программы задержки
extern char TxBuffer[BufferSize+1]; //Передающий Bluetooth буфер USART1
extern char RxBuffer[BufferSize+1]; //Приемный Bluetooth буфер USART1
extern uint16_t max[8], min[8] /*, timmin[8-1]*/;
//extern uint32_t error;
extern uint8_t index;
extern uint16_t typ_pin[13-1];
extern uint16_t DiviceStatus;
extern uint16_t TEMP;
extern const uint16_t VirtAddVarTab[NumbOfVar];
extern uint8_t thermostat_timer;
//*************Объявление испльзуемых функций***********************************
void delay_ms(uint16_t msec); //Задержка в милисекундах
void Dathiki(void);
//void Temperature(uint16_t temp); //функция сигнализации минимальной температуры
void SendString_InUnit(const char *str); //Функция отправки строки навнешний модуль через UART
void Reset_rxDMA_ClearBufer(void); //Сброс приемного DMA канала и очистка буфера приема
void LED(uint8_t led, LedMode_TypeDef state); //Функция управления светодиодами
void SET_ALARM(uint32_t time); //Установка будильника от текущего времени
void CLEAR_EEPROM(void); //Очистка всех настроек в EEPROM
uint8_t TEMP_InGrad(uint8_t dat);//Перевод температуры в градусы Цельсия
/***************************КОНЕЦ ФАЙЛА****************************************/