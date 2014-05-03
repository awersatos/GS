//******************************************************************************
//              �������� ������������ ���� �������
//******************************************************************************

//**************������������ �����**********************************************
#include "stm32f10x.h"
#include "eeprom.h"
//**************����������������************************************************
#define BufferSize 512       //������ �������    ����

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
//**************����������� ����������� �����***********************************

typedef enum //����������� ������ ������ ����������
{
OFF = 0,
RED,
GREEN,
ORANGE,
BLINK_RED,
BLINK_GREN,
BLINK_RED_GREEN
}LedMode_TypeDef;

//**************���������� ���������� ��������� �� ������ ������****************
extern LedMode_TypeDef Led1_state, Led2_state; //��������� �����������
extern uint32_t delay_Counter; //�������� ��������� ��������
extern char TxBuffer[BufferSize+1]; //���������� Bluetooth ����� USART1
extern char RxBuffer[BufferSize+1]; //�������� Bluetooth ����� USART1
extern uint16_t max[8], min[8] /*, timmin[8-1]*/;
//extern uint32_t error;
extern uint8_t index;
extern uint16_t typ_pin[13-1];
extern uint16_t DiviceStatus;
extern uint16_t TEMP;
extern const uint16_t VirtAddVarTab[NumbOfVar];
extern uint8_t thermostat_timer;
//*************���������� ����������� �������***********************************
void delay_ms(uint16_t msec); //�������� � ������������
void Dathiki(void);
//void Temperature(uint16_t temp); //������� ������������ ����������� �����������
void SendString_InUnit(const char *str); //������� �������� ������ ��������� ������ ����� UART
void Reset_rxDMA_ClearBufer(void); //����� ��������� DMA ������ � ������� ������ ������
void LED(uint8_t led, LedMode_TypeDef state); //������� ���������� ������������
void SET_ALARM(uint32_t time); //��������� ���������� �� �������� �������
void CLEAR_EEPROM(void); //������� ���� �������� � EEPROM
uint8_t TEMP_InGrad(uint8_t dat);//������� ����������� � ������� �������
/***************************����� �����****************************************/