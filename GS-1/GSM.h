//******************************************************************************
//               ЗАГОЛОВОЧНЫЙ ФАЙЛ БИБЛИОТЕКИ GSM МОДУЛЯ
//******************************************************************************

//**************Подключаемые файлы**********************************************
#include "stm32f10x.h"

//**************Макроопределения************************************************
#define GSM_MOD GPIOA

#define GSM_ON GPIO_Pin_8
#define GSM_RESET GPIO_Pin_11
#define GSM_READY GPIO_Pin_12

#define GSM_RING GPIO_Pin_9

#define Beeline_OP 1
#define MTS_OP 2
#define Megafon_OP 3


//**************Определение собственных типов***********************************
typedef enum //Определение типа смс
{
NUMBER_SET,
RESET_OK,
COMMAND_COMPLETE,
COMMAND_ERROR,
GAS_ALARM,
TEMP_ALARM,
TEMP2,
VOLTAGE_MISSING,
VOLTAGE_MISSING_3H,
VOLTAGE_ON,
SMOK,
FIRE,
INVASION,
LOW_BAT,
OPTIONS,
COTEL_BLOK,
HOME_TEMP

}SMS_TypeDef;
//**************Глобальные переменные доступные из других файлов****************
extern char IMEI[]; //Массив для IMEI
extern uint8_t OPERATOR; //Оператор сотовой связи
extern uint8_t SendDataError; //Ошибка передачи данных
//*************Объявление испльзуемых функций***********************************
void GSM_Configuration(void); //Инициализация GSM
void SEND_SMS(SMS_TypeDef sms, uint8_t number); //Функция отправки СМС
void RECEIVE_SMS(void); //Функция получения СМС сообщения
void SendData_onServer(SMS_TypeDef sms);  //Функция отправки данных на сервер
void BALLANSE(uint8_t nbr); //Функция проверки балланса
/***************************КОНЕЦ ФАЙЛА****************************************/