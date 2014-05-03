/*******************************************************************************
********************************************************************************
**                                                                            **
**                  �������� ���� ������� GS-1                                **
**                                                                            **
********************************************************************************
*******************************************************************************/


//****************������������ �����********************************************
#include "stm32f10x_conf.h"
#include "eeprom.h"
#include "stm32f10x.h"
#include "main.h"
#include "GSM.h"
#include <string.h>
//************* ������������� ���������� ���������� ****************************
uint32_t delay_Counter; //�������� ��������� ��������

char TxBuffer[BufferSize+1]; //���������� Bluetooth ����� USART1
char RxBuffer[BufferSize+1]; //�������� Bluetooth ����� USART1

uint16_t error = 0; //������ �� �������� ����������� ������� � �������
uint16_t error_stat = 0;
uint16_t adc_buffer[11]; //
uint16_t typ_pin[13-1]; //0-��������� >1-������

LedMode_TypeDef Led1_state, Led2_state; //��������� �����������

uint16_t TEMP; //��������� ����������

uint16_t DiviceStatus = 0; //������ �������� ������

uint16_t temp_mem, max, min[16] /*, timmin[8-1]*/ ;

uint8_t count, index, state;

const uint32_t temperlim = 100;

uint8_t thermostat_timer =0;

uint8_t t1,t2;





//������ ����������� ������� EEPROM
const uint16_t VirtAddVarTab[NumbOfVar] = 
{ 0xAB00, 0xAB01, 0xAB02, 0xAB03, 0xAB04, 0xAB05,0xAB06, 0xAB07, 0xAB08, 0xAB09, 0xAB0A, 0xAB0B, 0xAB0C, 0xAB0D, 0xAB0E, 0xAB0F,
  
  0xAB10, 0xAB11, 0xAB12, 0xAB13, 0xAB14, 0xAB15,0xAB16, 0xAB17, 0xAB18, 0xAB19, 0xAB1A, 0xAB1B, 0xAB1C, 0xAB1D, 0xAB1E, 0xAB1F,
  
  0xAB20, 0xAB21, 0xAB22, 0xAB23, 0xAB24, 0xAB25,0xAB26, 0xAB27, 0xAB28, 0xAB29, 0xAB2A, 0xAB2B, 0xAB2C, 0xAB2D, 0xAB2E, 0xAB2F,
    
  0xAB30, 0xAB31, 0xAB32, 0xAB33, 0xAB34, 0xAB35,0xAB36, 0xAB37, 0xAB38, 0xAB39, 0xAB3A, 0xAB3B, 0xAB3C, 0xAB3D, 0xAB3E, 0xAB3F
};


//******************��������� �������*******************************************
static void RCC_Configuration(void); //��������� ����������� � ��������� ������������� ���������
static void GPIO_Configuration(void); //������������� ������ ����� ������
static void EXTI_Configuration(void); //������������� ����������� ������� ����������
static void NVIC_Configuration(void);//������������� ����������
static void ADC_Configuration(void);//������������� ���
static void DMA_Configuration(void); //������������� ������� DMA
static void TIMER_Configuration(void);//������������� ��������
static void UART_Configuration(void);    //������������� USART
static void ClearBufer(char *buf); //������� ������� ������
static void BKP_Configuration(void); //������������ ���������� ������ �������
static void RTC_Configuration(void); //������������ ����� ��������� �������
static void ERROR_EXEC(void);
static void OUT_EXEC(void); //���������� �������� ��������
static void Thermostat(void); //������� ����������
static void TEMP_CTRL(uint16_t temp); //������� �������� ����������� �������������
//******************************************************************************


//==============================================================================
                  /*������ ������������ ���� ���������*/
//==============================================================================


void main() //�������� ������ ���������
{
          /*������������� ������������ ��������� �����������*/  
FLASH_Unlock(); //������������� ����������� ���� ������
EE_Init(); //������������� ������������ EEPROM
RCC_Configuration(); //��������� ����������� � ��������� ������������� ���������
BKP_Configuration(); //������������ ���������� ������ ��������
RTC_Configuration(); //������������ ����� ��������� �������
GPIO_Configuration(); //������������� ������ ����� ������
EXTI_Configuration(); //������������� ����������� ������� ����������
ADC_Configuration();//������������� ���
DMA_Configuration(); //������������� ������� DMA
TIMER_Configuration();//������������� ��������
UART_Configuration();//������������� USART
NVIC_Configuration();//������������� ����������
SysTick_Config(SystemCoreClock/ 1000); //���������� ���������� ������� 1��
NVIC_SetPriority(SysTick_IRQn,0); //������������������� ���������� ������� ���������





LED(1, ORANGE);
LED(2, ORANGE);

GSM_Configuration(); //���������������� ������

index=0;
count=0;
state=0;
max=adc_buffer[8]; //PB0
min[0]=adc_buffer[8]; //PB0
temp_mem=adc_buffer[8];





EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
TEMP &= (1<<15);

if(TEMP == 0)
{
 LED(1, BLINK_RED_GREEN);
 LED(2, BLINK_RED_GREEN); 
 CLEAR_EEPROM(); //������� ���� �������� � EEPROM
 DiviceStatus |= 0x01; 
 SET_ALARM(120);
 while((DiviceStatus & 0x01) == 1);
 SendString_InUnit("AT+CMGD=0,4\r\n"); 
 delay_ms(100);
 Reset_rxDMA_ClearBufer(); //����� ������  
}
SendString_InUnit("AT+CMGD=0,4\r\n");

//SendData_onServer(FIRE);  //������� �������� ������ �� ������


LED(1, GREEN);
LED(2, GREEN);

//BALLANSE(PHONE1); //������� �������� ��������
//SEND_SMS(HOME_TEMP, PHONE1);


  while(1)//�������� ���� ���������
  {
   RECEIVE_SMS(); //������� ��������� ��� ���������
   delay_ms(1000);
   Dathiki();
  TEMP_CTRL(adc_buffer[8]);
  
  
  ERROR_EXEC();
  OUT_EXEC();
  if(thermostat_timer > 60)
  {
   Thermostat();
   thermostat_timer =0;
  }
  t1 = TEMP_InGrad(1);
  t2 = TEMP_InGrad(2);
  
  
  }//����� ��������� �����
}
//==============================================================================
             /*����� �������� ������� ���������*/
//==============================================================================
static void RCC_Configuration(void) //��������� ����������� � ��������� ������������� ���������
{
 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);  
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//��������� ������������ ������ ������� � ���������� ������
 RCC_LSEConfig(RCC_LSE_OFF );
 RCC_LSICmd(ENABLE); //��������� ����������� RC ����������
 while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); //�������� ���������� ����������� RC ����������

}
//==============================================================================
void BKP_Configuration(void) //������������ ���������� ������ �������
{
PWR_BackupAccessCmd(ENABLE); //���������� ������ � �������� � ��������������� �������
BKP_DeInit();

BKP_TamperPinCmd(DISABLE); //��� ������� ��������
 
}
//==============================================================================
void RTC_Configuration(void) //������������ ����� ��������� �������
{
  
 RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); //�������� ��������� ������� ���������� RC ���������
 RCC_RTCCLKCmd(ENABLE); //��������� ������������ �����

 RTC_WaitForSynchro(); //�������� �������������
 RTC_WaitForLastTask(); //�������� ��������� ������ � �������� 
 RTC_SetPrescaler(40000);//��������� ������������
 RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
 
 RTC_ITConfig(RTC_IT_SEC, ENABLE); //���������� ������ �������
 RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
 
 RTC_ITConfig(RTC_IT_ALR, ENABLE); //���������� �����
 RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
}

//==============================================================================
static void GPIO_Configuration(void) //������������� ������ ����� ������
{
 GPIO_InitTypeDef GPIO_InitStructure;

//ADC_Channel_0-PA0, 1-A1, 2-A2, 3-A3, 4-A4, 5-A5, 6-A6,7-A7, 8-B0, 9-B1, 10-C0, 11-C1, 12-C2, 13-C3, 14-C4, 15-C5
// ����������� ��� �� ������ � ������ ����������� �����
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //������ �������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //������ �������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //������ �������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //������ �������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //������ �������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //������ �������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //������� � ����� ����������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //������� � ����� ���������������
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //�����������
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //�����������
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //������� 12�
     GPIO_Init(GPIOC, &GPIO_InitStructure);

     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //��������� ������� 220�
     GPIO_Init(GPIOC, &GPIO_InitStructure);


     // ����������� ���� PA10 ��� ���� UART� (RxD)
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     // ����������� ���� PA9 ��� ����� UART� (TxD)
     // ������ �� ������ �����, � ����� � �������������� ��������
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     //PA8-on/off P11-reset P12-ready PC9-ring
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
     GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_SetBits(GSM_MOD , GSM_ON);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_SetBits(GSM_MOD , GSM_RESET);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
     GPIO_Init(GPIOC, &GPIO_InitStructure);
     
     //PB12 13 14 15 out
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
     GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
     GPIO_Init(GPIOB, &GPIO_InitStructure);


     //PB5 PB6 ���������� �� 220/40 � 12�
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
     GPIO_Init(GPIOB, &GPIO_InitStructure);

     //PC2 PC3 PC4 PC5 ����������
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
     GPIO_Init(GPIOC, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
     GPIO_Init(GPIOC, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
     GPIO_Init(GPIOC, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
     GPIO_Init(GPIOC, &GPIO_InitStructure); 
     
     GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
     GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
     GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
     GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9);
}
//==============================================================================
static void EXTI_Configuration(void) //������������� ����������� ������� ����������
{
  EXTI_InitTypeDef EXTI_InitStructure;
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line6;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line9;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}
//==============================================================================
static void NVIC_Configuration(void)//������������� ����������
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//����������� ����� �����������:2 ���������
  NVIC_InitTypeDef NVIC_InitStruct; //��������� ��������� ��������� ����������
  
  NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn; //���������� ������� ����� 5-9
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //������������ ���������
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 5; //���������
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
  NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������
  
  NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn; //���������� ������� ����� 5-9
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //������������ ���������
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //���������
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
  NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������
  
  NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  
  NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  
  NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);



  
}
//==============================================================================
static void ADC_Configuration(void)//������������� ���
{
     ADC_InitTypeDef ADC_InitStructure;

     ADC_StructInit(&ADC_InitStructure);
    
     ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //����� � �� ��������(�������� �������), ����������
     ADC_InitStructure.ADC_ScanConvMode = ENABLE;//DISABLE;
     ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //��� �����������
     ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//T3_TRGO;
     ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
     ADC_InitStructure.ADC_NbrOfChannel = 11;
     ADC_Init(ADC1, &ADC_InitStructure);
  
     // �������� ������ ������� ������ ���
     ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5); //PA0 = adc_buffer[0]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5); //PA1 = adc_buffer[1]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5); //PA2 = adc_buffer[2]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_239Cycles5); //PA3 = adc_buffer[3]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_239Cycles5); //PA4 = adc_buffer[4]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_239Cycles5); //PA5 = adc_buffer[5]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_239Cycles5); //PA6 = adc_buffer[6]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_239Cycles5); //PA7 = adc_buffer[7]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 9, ADC_SampleTime_239Cycles5); //PB0 = adc_buffer[8]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 10, ADC_SampleTime_239Cycles5); //PB1 = adc_buffer[9]
     ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 11, ADC_SampleTime_239Cycles5); //PC0 = adc_buffer[9]
     
     // �������� ���
     ADC_DMACmd(ADC1, ENABLE);
     ADC_Cmd(ADC1, ENABLE);
     
     // ���������� ���
     ADC_ResetCalibration(ADC1); //����� ����������
     while (ADC_GetResetCalibrationStatus(ADC1));
     ADC_StartCalibration(ADC1);
     while (ADC_GetCalibrationStatus(ADC1));
     ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
  
  
}
//==============================================================================
static void DMA_Configuration(void) //������������� ������� DMA
{
     DMA_InitTypeDef DMA_InitStructure;
  
     DMA_StructInit(&DMA_InitStructure);
  
     DMA_DeInit(DMA1_Channel1);     // ������ ����� ����� �� �������� ������ ADC1
     DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
     DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_buffer; // ������������ ������ ����� � ����������
     DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  // �������� ������ �� ��������� � ������
     DMA_InitStructure.DMA_BufferSize = 11; // ������ ������
     DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// ����� ��������� ������ �� ��������������
     DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;// ������ ����������������
     DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // ��������� ������� ������//16���
     DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
     DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;    //DMA_Mode_Normal;
     DMA_InitStructure.DMA_Priority = DMA_Priority_High;
     DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
     DMA_Init(DMA1_Channel1, &DMA_InitStructure);
     
       //���������� �� ���������� � ��������������
     DMA_ITConfig(DMA1_Channel1, DMA_IT_TC /*| DMA_IT_HT*/, ENABLE);
         
     DMA_Cmd(DMA1_Channel1, ENABLE);// �������� ������ ����� DMA1
     
     /*USART1_TX-����� 4*/
  DMA_DeInit(DMA1_Channel4);  //����� �������� ������ 4
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //������� ����� �������� ������ USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBuffer; //���������� ����� USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //����������� ������ �� ������ � ���������
  DMA_InitStructure.DMA_BufferSize = BufferSize; //������ ������
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //��������� ������������� �������� ��������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //��������� ������ �������� 
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //������ ������ � ��������� 1 ����
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //������ ������ � ������ 1 ����
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //���������� ����� ������ DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //���������  ������
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //�������� �� ������ � ������ ���������
  
  DMA_Init(DMA1_Channel4 , &DMA_InitStructure); //�������� ��������� � �������
 // DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE); //��������� ���������� �� ��������� ��������
  
  /*USART1_RX-�����5*/
  DMA_DeInit(DMA1_Channel5);  //����� �������� ������ 5
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //������� ����� �������� ������ USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBuffer; //�������� ����� USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //����������� ������ �� ��������� � ������
  DMA_InitStructure.DMA_BufferSize = BufferSize; //������ ������
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //����������� ����� ������ DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //���������  �������
  
  DMA_Init(DMA1_Channel5 , &DMA_InitStructure); //�������� ��������� � �������
  
  DMA_Cmd(DMA1_Channel5, ENABLE);//USART1_RX

}
//==============================================================================
static void TIMER_Configuration(void)//������������� ��������
{
     TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   
     TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
       //24000000 ������� 72000000
     TIM_TimeBaseStructure.TIM_Period = 2000;       //������ ������ 1000 ��� � ������� (���� ������� 24000000��)
     TIM_TimeBaseStructure.TIM_Prescaler = 36000 - 1;
     TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
     TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
     TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 
     
     TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update); // ����� �������������
     
     
     TIM_Cmd(TIM3, ENABLE);// ������ �������
     
     TIM_TimeBaseStructure.TIM_Period = 500;       //������ ������ 1000 ��� � ������� 
     TIM_TimeBaseStructure.TIM_Prescaler = 36000 ;
     TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
     TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
     TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); 
     
     TIM_ITConfig(TIM4 , TIM_IT_Update , ENABLE); //������ �� ���������� �� ������������ //��������� ���������� �� �������
     TIM_Cmd(TIM4, ENABLE);// ������ ������� 

}
//==============================================================================
static void UART_Configuration(void)    //������������� USART
{
 USART_InitTypeDef USART_InitStructure; //���������� ��������� ������������
 USART_StructInit(&USART_InitStructure); //������� ����� ���������
                   /*USART1*/
  USART_InitStructure.USART_BaudRate = 115200;  //�������� 115200 �/�
  USART_InitStructure.USART_WordLength = USART_WordLength_8b; //���� ������ 8
  USART_InitStructure.USART_StopBits = USART_StopBits_1;  //�������� 1
  USART_InitStructure.USART_Parity = USART_Parity_No;     //�������� ���
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //����������� ������� ���
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //���������� � �������� ��������
  
  USART_Init(USART1, &USART_InitStructure); //�������� ��������� � �������
  
  USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //��������� ������ DMA �� ����� � ��������
  USART_Cmd(USART1, ENABLE); //������������ USART1
}
//==============================================================================
void Dathiki(void) //������ ��������
{
  
  error=0;
  EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
  typ_pin[0] = TEMP & 0x000F;
  typ_pin[1] = TEMP & 0x00F0;
  typ_pin[2] = TEMP & 0x0F00;
  typ_pin[3] = TEMP & 0xF000;  
  EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
  typ_pin[4] = TEMP & 0x000F;
  typ_pin[5] = TEMP & 0x00F0;
  typ_pin[6] = TEMP & 0x0F00;
  typ_pin[7] = TEMP & 0xF000;
  
   if (typ_pin[0] != 0) {
     if (adc_buffer[0] < (adc_buffer[10]-500)) error|= 1;
   }
   if (typ_pin[1] != 0) {
     if (adc_buffer[1] < (adc_buffer[10]-500)) error|= (1 << 1);
   }
   if (typ_pin[2] != 0) {
     if (adc_buffer[2] < (adc_buffer[10]-500)) error|= (1 << 2);
   }
   if (typ_pin[3] != 0) {
     if (adc_buffer[3] < (adc_buffer[10]-500)) error|= (1 << 3);
   }
   if (typ_pin[4] != 0) {
     if (adc_buffer[4] < (adc_buffer[10]-500)) error|= (1 << 4);
   }
   if (typ_pin[5] != 0) {
     if (adc_buffer[5] < (adc_buffer[10]-500)) error|= (1 << 5);
   }
   if (typ_pin[6] != 0) {
     if (adc_buffer[6] < (adc_buffer[10]-500)) error|= (1 << 6);
   }
   if (typ_pin[7]!= 0) {
     if (adc_buffer[7] < (adc_buffer[10]-500)) error|= (1 << 7);
   }
   
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((uint8_t)TEMP) !=((uint8_t) error))
   {
     TEMP&=0xFF00;
     TEMP|=error;
     EE_WriteVariable(VirtAddVarTab[ERROR], TEMP);
   }
   
} 
//==============================================================================
/*
void Temperature(uint16_t temp) //������� ������������ ����������� �����������
{
if (temp > max[index]) max[index]=temp; //��������� �������� �������� �����
if (temp < min[index]) min[index]=temp; //��������� ������� �������� �����

if (count > 0) //���� � ��� ��� ������ ������ ����
   {
     if (max[(index-1)&3]-min[(index-1)&3]*0.15 <= min[(index-1)&3]-min[index]) error |= 1; //������ ����������� ���������� � ���� �������� ��������
     if ((state == 1) & (temp <= (min[(index-1)&3]+((max[(index-1)&3]-min[(index-1)&3])*0.3)))) state = 1; //����� max � ����� ����� �� min
     if ((state == 0) & (temp >= (min[(index-1)&3]+((max[(index-1)&3]-min[(index-1)&3])*0.7)))) state = 2; //����� max � min ����� ����� �� max
     if (state == 2)
     {
       if (count < 7) count++;
       index= (index+1)&3; //���� �������� ��������� 1 � �������� ��� ���� ����� 2, �.�. ����� �� 8 � �� ���������
       state = 0;
       max[index]=temp;
       min[index]=temp;
     }
   }
else //���� � ��� ��� �� ������ ������ ����
   {
     if (max[index] - min[index] > temperlim) 
                //���� ���� ��������
     {
       if ((state == 1) & (temp <= (max[index]+min[index])*0.5)) state = 3; //����� ������� �� max ��� ������ � �������� ��������
       if ((state == 2) & (temp >= (max[index]+min[index])*0.5)) state = 4; //����� ��������� �� min ��� ����������� � �������� ��������
       if ((state == 3) & (temp >= (min[index]+((max[index]-min[index])*0.7)))) state = 5; //������ ����� min � ����� ����� �� max
       if ((state == 4) & (temp <= (min[index]+((max[index]-min[index])*0.3)))) state = 6; //������ ����� max � ����� ����� �� min
       if ((state == 5) & (temp <= (min[index]+((max[index]-min[index])*0.3)))) state = 6; //����� ����� �� min
       if (state == 6)
       {
         count++; //������ �� ���������� ���� ����� � ��������� ������������ ����������� �����������
         index++;
         state = 0;
         max[index]=temp;
         min[index]=temp;
       }
     }
   }
}
*/
//==============================================================================
void delay_ms(uint16_t msec) //�������� � ������������
{
 delay_Counter = msec;
 while(delay_Counter != 0);
  
}
//==============================================================================
void SendString_InUnit(const char *str) //������� �������� ������ ��������� ������ ����� UART
{
 uint16_t len; //������ ������
 uint16_t i;   //�������
 len=strlen(str); //��������� ������ ������
 
 for(i=0;i<len;i++)  TxBuffer[i]=*str++; //��������� ������ � ����� ��������
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //��������� ����� DMA
  DMA_SetCurrDataCounter(DMA1_Channel4 , len);      //������������� ������� DMA ������
  DMA_ClearFlag(DMA1_FLAG_TC4);                     //���������� ���� ��������� ��������
  DMA_Cmd(DMA1_Channel4 , ENABLE );                 //���������� DMA �����
  while(DMA_GetFlagStatus(DMA1_FLAG_TC4)==RESET);   //���� ��������� ��������
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //��������� ����� DMA
  ClearBufer(TxBuffer);                   //������� ����� 
}
//==============================================================================
static void ClearBufer(char *buf) //������� ������� ������
{
 uint16_t j; //������� 
 for(j=0;j<BufferSize;j++) 
 {
   *buf=0x00; //����������� �������� �������� �������� �� ���������
   buf++; //�������������� ��������� ����������
 }
}
//==============================================================================
void Reset_rxDMA_ClearBufer(void) //����� ��������� DMA ������ � ������� ������ ������
{
     DMA_Cmd(DMA1_Channel5 , DISABLE ); //������������� DMA �����
     ClearBufer(RxBuffer);                  //������� �����
     DMA_SetCurrDataCounter(DMA1_Channel5 , BufferSize); //������ ������� � ������ ������
     DMA_Cmd(DMA1_Channel5 , ENABLE ); //���������� DMA �����
 
  
}
//==============================================================================
void LED(uint8_t led, LedMode_TypeDef state) //������� ���������� ������������
{
  switch( state)
  {
  case OFF:
    if(led == 1)
    {
     Led1_state =  OFF;
     GPIO_ResetBits(GPIOC, GPIO_Pin_2);
     GPIO_ResetBits(GPIOC, GPIO_Pin_3);
     
    }
     if(led == 2)
    {
     Led2_state =  OFF;
     GPIO_ResetBits(GPIOC, GPIO_Pin_4);
     GPIO_ResetBits(GPIOC, GPIO_Pin_5);    
    }
   break; 
  
   case RED:
     if(led == 1)
    {
     Led1_state =  RED;
     GPIO_SetBits(GPIOC, GPIO_Pin_2);
     GPIO_ResetBits(GPIOC, GPIO_Pin_3);
     
    }
     if(led == 2)
    {
     Led2_state =  RED;
     GPIO_SetBits(GPIOC, GPIO_Pin_4);
     GPIO_ResetBits(GPIOC, GPIO_Pin_5);    
    }
   break; 
   
  
   case GREEN:
     if(led == 1)
    {
     Led1_state =  GREEN;
     GPIO_ResetBits(GPIOC, GPIO_Pin_2);
     GPIO_SetBits(GPIOC, GPIO_Pin_3);
     
    }
     if(led == 2)
    {
     Led2_state =  GREEN;
     GPIO_ResetBits(GPIOC, GPIO_Pin_4);
     GPIO_SetBits(GPIOC, GPIO_Pin_5);    
    }
   break; 
   
   case ORANGE:
     if(led == 1)
    {
     Led1_state =  ORANGE;
     GPIO_SetBits(GPIOC, GPIO_Pin_2);
     GPIO_SetBits(GPIOC, GPIO_Pin_3);
     
    }
     if(led == 2)
    {
     Led2_state =  ORANGE;
     GPIO_SetBits(GPIOC, GPIO_Pin_4);
     GPIO_SetBits(GPIOC, GPIO_Pin_5);    
    }
   break;
   
   case BLINK_RED:
     if(led == 1)Led1_state =  BLINK_RED;
     if(led == 2)Led2_state =  BLINK_RED;
     break;
     
   case BLINK_GREN:
     if(led == 1)Led1_state =  BLINK_GREN;
     if(led == 2)Led2_state =  BLINK_GREN;
     break;
     
   case BLINK_RED_GREEN:
     if(led == 1)Led1_state =  BLINK_RED_GREEN;
     if(led == 2)Led2_state =  BLINK_RED_GREEN;
     break;  
  }
  
}
//==============================================================================
void SET_ALARM(uint32_t time) //��������� ���������� �� �������� �������
{
  uint32_t time_now;
  
  time_now = RTC_GetCounter(); //���������� �������
  RTC_SetAlarm(time_now+time);
  RTC_WaitForLastTask(); //�������� ��������� ������ � ��������

}
//==============================================================================
void CLEAR_EEPROM(void) //������� ���� �������� � EEPROM
{
EE_WriteVariable(VirtAddVarTab[ERROR], 0x0000); 
EE_WriteVariable(VirtAddVarTab[ERROR_SATUS], 0x0000);
EE_WriteVariable(VirtAddVarTab[SL_ST1], 0x0000);
EE_WriteVariable(VirtAddVarTab[SL_ST2], 0x0000);
EE_WriteVariable(VirtAddVarTab[SETTINGS], 0x0000);
EE_WriteVariable(VirtAddVarTab[Q1], 0x0000);
EE_WriteVariable(VirtAddVarTab[Q2], 0x0000);
EE_WriteVariable(VirtAddVarTab[SETTINGS_2], 0x0000);
EE_WriteVariable(VirtAddVarTab[TERM], (((25*125)/10)+610));
EE_WriteVariable(VirtAddVarTab[GIST], ((3*125)/10));
for(uint8_t i=0;i<6;i++) EE_WriteVariable(VirtAddVarTab[PHONE1+i], 0x0000);
for(uint8_t i=0;i<6;i++) EE_WriteVariable(VirtAddVarTab[PHONE2+i], 0x0000);
}
//==============================================================================
static void ERROR_EXEC(void)
{
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if((TEMP & 0x03FF) !=0) 
   {LED(2, BLINK_RED);
  // GPIO_SetBits(GPIOB, GPIO_Pin_12);
   }
   else LED(2, GREEN);
   
   if(((TEMP & 1) !=0) && ((error_stat & 1) ==0)) //�����-1
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     TEMP&=0x000F;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);                  
         error_stat|=1;
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;
                  
         
       case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;    
         
       case 3:        
         SEND_SMS(INVASION, PHONE1);
         DiviceStatus |= (1<<5);
         break;
       }
          error_stat|=1;
   }
 //------------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((TEMP & (1<<1)) !=0) && ((error_stat & (1<<1)) ==0)) //�����-2
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     TEMP&=0x00F0;
     TEMP = TEMP>>4;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;
         
      case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break; 
      case 3:        
         SEND_SMS(INVASION, PHONE1);
         DiviceStatus |= (1<<5);
         break;
       }
       error_stat|=(1<<1);
   } 
   
 //------------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((TEMP & (1<<2)) !=0) && ((error_stat & (1<<2)) ==0)) //�����-3
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     TEMP&=0x0F00;
     TEMP = TEMP>>8;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
        
         break;
         
      case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;
      case 3:        
         SEND_SMS(INVASION, PHONE1); 
         DiviceStatus |= (1<<5);
         break;         
       }
       error_stat|=(1<<2);
   } 
//------------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((TEMP & (1<<3)) !=0) && ((error_stat & (1<<3)) ==0)) //�����-4
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     TEMP&=0xF000;
     TEMP = TEMP>>12;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);// error_stat|=1;
         break;
         
      case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1); 
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);         
         break; 
      case 3:        
         SEND_SMS(INVASION, PHONE1); 
         DiviceStatus |= (1<<5);
         break;         
       }
       error_stat|=(1<<3);
   }      
//------------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((TEMP & (1<<4)) !=0) && ((error_stat & (1<<4)) ==0)) //�����-5
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     TEMP&=0x000F;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;
         
      case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break; 
         
       case 3:        
         SEND_SMS(INVASION, PHONE1); 
         
         DiviceStatus |= (1<<5);
         break;         
         
       }
       error_stat|=(1<<4);
   } 

//------------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((TEMP & (1<<5)) !=0) && ((error_stat & (1<<5)) ==0)) //�����-6
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     TEMP&=0x00F0;
     TEMP = TEMP>>4;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;
         
      case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break; 
         
       case 3:        
         SEND_SMS(INVASION, PHONE1); 
         
         DiviceStatus |= (1<<5);
         break; 
         
       case 4:        
         SEND_SMS(COTEL_BLOK, PHONE1); 
         
        // DiviceStatus |= (1<<5);
         break;           
       }
       error_stat|=(1<<5);
   }
 //------------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((TEMP & (1<<6)) !=0) && ((error_stat & (1<<6)) ==0)) //�����-7
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     TEMP&=0x0F00;
     TEMP = TEMP>>8;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;
         
      case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break; 
         
       case 3:        
         SEND_SMS(INVASION, PHONE1);
         
         DiviceStatus |= (1<<5);
         break;         
       }
       error_stat|=(1<<6);
   } 
//------------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if(((TEMP & (1<<7)) !=0) && ((error_stat & (1<<7)) ==0)) //�����-8
   {
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     TEMP&=0xF000;
     TEMP = TEMP>>12;
       switch(TEMP)
       {
       case 1:
         SendData_onServer(SMOK);
         delay_ms(1500);
         SEND_SMS(SMOK, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break;
         
      case 2:
         SendData_onServer(FIRE);
         delay_ms(1500);
         SEND_SMS(FIRE, PHONE1);                  
         DiviceStatus |= (1<<4);
         DiviceStatus |= (1<<5);
         break; 
         
       case 3:        
         SEND_SMS(INVASION, PHONE1); 
         
         DiviceStatus |= (1<<5);
         break;         
         
       case 4:
         SendData_onServer(GAS_ALARM);
         delay_ms(1500);
         SEND_SMS(GAS_ALARM, PHONE1);                  
         DiviceStatus |= (1<<3);
         DiviceStatus |= (1<<5);
         break;    
         
       }
       error_stat|=(1<<7);
   }
 //-----------------------------------------------------------------------------
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); //������ ����
   if(((TEMP & (1<<8)) !=0) && ((error_stat & (1<<8)) ==0))
   {
     EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     if((TEMP & 0x03) !=0)
     {
         SendData_onServer(GAS_ALARM);
         delay_ms(1500);
         SEND_SMS(GAS_ALARM, PHONE1);
         DiviceStatus |= (1<<3);
         DiviceStatus |= (1<<5);
     }
     error_stat|=(1<<8);      
   }
 //-----------------------------------------------------------------------------
  EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); //����������� �������� ����������� ������������� 
  if(((TEMP & (1<<9)) !=0) && ((error_stat & (1<<9)) ==0))
  {
   SendData_onServer(TEMP_ALARM);
   delay_ms(1500);
   SEND_SMS(TEMP_ALARM, PHONE1);
    
    error_stat|=(1<<9);
  }
//------------------------------------------------------------------------------ 
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); //���������� ���������� ����� 3 �����
  if(((TEMP & (1<<11)) !=0) && ((error_stat & (1<<11)) ==0))
  {
   SendData_onServer(VOLTAGE_MISSING_3H);
   delay_ms(1500);
   SEND_SMS(VOLTAGE_MISSING_3H, PHONE1);
    
    error_stat|=(1<<11);
  }
//------------------------------------------------------------------------------ 
     EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); //���������� ���������� ���������������
  if(((TEMP & (1<<12)) !=0) && ((error_stat & (1<<12)) ==0))
  {
   EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
   if((TEMP & (1<<11)) !=0)
   {
   SendData_onServer(VOLTAGE_MISSING);
   delay_ms(1500);
   SEND_SMS(VOLTAGE_MISSING, PHONE1);
   }
    error_stat|=(1<<12);
  }
//------------------------------------------------------------------------------ 
     EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); //����������� ������ �������
  if(((TEMP & (1<<13)) !=0) && ((error_stat & (1<<13)) ==0))
  {   
   SendData_onServer(LOW_BAT);
   delay_ms(1500);
   SEND_SMS(LOW_BAT, PHONE1);
   
    error_stat|=(1<<13);
  }
//------------------------------------------------------------------------------  
}
//==============================================================================
uint8_t TEMP_InGrad(uint8_t dat)//������� ����������� � ������� �������
{
  uint16_t temp;
  
  if(dat == 1) temp = (adc_buffer[8] - 610)*10;   //625
  else if(dat == 2) temp = (adc_buffer[9] - 610)*10;  //625
   temp = temp/125;
   return (uint8_t) temp;
  
}
//==============================================================================
static void OUT_EXEC(void) //���������� �������� ��������
{
  EE_ReadVariable(VirtAddVarTab[SETTINGS_2], &TEMP);
  for(uint8_t i=1; i<6;i++)
  {
    if((TEMP & 0x000F) == i) //OUT1
    {
      if((DiviceStatus & (1<<(i+2))) != 0) GPIO_SetBits(GPIOB, GPIO_Pin_12);
      else GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    }
//------------------------------------------------------------------------------
    if(((TEMP & 0x00F0)>>4) == i) //OUT2
    {
      if((DiviceStatus & (1<<(i+2))) != 0) GPIO_SetBits(GPIOB, GPIO_Pin_13);
      else GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    }    
//------------------------------------------------------------------------------
    if(((TEMP & 0x0F00)>>8) == i) //OUT_Relay1
    {
      if((DiviceStatus & (1<<(i+2))) != 0) GPIO_SetBits(GPIOB, GPIO_Pin_14);
      else GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    }    
//------------------------------------------------------------------------------
    if(((TEMP & 0xF000)>>12) == i) //OUT_Relay2
    {
      if((DiviceStatus & (1<<(i+2))) != 0) GPIO_SetBits(GPIOB, GPIO_Pin_15);
      else GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    }      
  }
  
}
//==============================================================================
static void Thermostat(void) //������� ����������
{
  uint16_t term;
  EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
  if((TEMP & (1<<3)) == 0) return;
  else
  {
   EE_ReadVariable(VirtAddVarTab[TERM], &term); 
   if((adc_buffer[9] >= term)) //���� ����������� ��������� ���� �������������
   {DiviceStatus |= (1<<7);
    if((DiviceStatus & (1<<8)) != 0) DiviceStatus &= ~(1<<8);
   }
   else
   {
    EE_ReadVariable(VirtAddVarTab[GIST], &TEMP);
    if(adc_buffer[9] <= (term-TEMP)) DiviceStatus &= ~(1<<7);//���� ����������� ����� ���� �����������
    if(adc_buffer[9] < (term-87))
    {
     if((DiviceStatus & (1<<8)) == 0)
     {
      SEND_SMS(HOME_TEMP, PHONE1);
      DiviceStatus |= (1<<8);
     }
    }
   }
   
  }
  
}
//==============================================================================
static void TEMP_CTRL(uint16_t temp) //������� �������� ����������� �������������
{
 if (temp > max) max=temp; //��������� �������� �������� �����
 if (temp < min[index]) min[index]=temp; //��������� ������� �������� �����

 if(count > 10) //������ 10 ������� ������� �������� ���� �����������
 {
   if(temp > temp_mem) //���� ����� �����������
   {state &= ~(1<<1);
    state |= 0x01;}
   else if(temp < temp_mem) //���� ������� �����������
     {state &= ~0x01;
      state |= (1<<1);}
  temp_mem = temp;
  count = 0;
 }
 else count++;
 
 if(((state & 0x03) == 1) && (temp < (max + min[index])/2) && (temp > (max + min[index])/4)) state |= (1<<2); //��������� �������
 else if(((state & 0x03) == 2) && (temp > (max + min[index])/2)&& (temp < ((max + min[index])/4)*3)) state |= (1<<3); //��������� ��������
 
 if(((state & 0x0C)>>2) == 3) //���� ������������� ������� � �������� ������� �� ��������� ����
 {
   state &= 0x80;
   index++;
   if(index > 15)
   {
    index = 0;
    state |= 0x80; //����� �������� ����� �������������
   }
   max=temp;
   min[index]=temp;
 }
 
 EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
 if((state & 0x80) != 0) //������� ������ ����������� � �������� ������
 {
   uint32_t TempThreshold =0;//����� ������ �����������
   for(uint8_t i=0;i<16;i++) if(i != index) TempThreshold = TempThreshold+min[i]; //������� ����� ���������
   TempThreshold = TempThreshold/15; //������� ������� �����������
   TempThreshold = TempThreshold -(TempThreshold/10); //������������� �������� ������ �� 10% ���� ������������������
    if(temp < TempThreshold) //���� ����������� ���� ������ ��������� ������
    {
     // EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
      if((TEMP & (1<<9)) ==0)
      {
        TEMP |= (1<<9);
        EE_WriteVariable(VirtAddVarTab[ERROR], TEMP);
      }
    }
    else if((TEMP & (1<<9)) !=0)
   {
    TEMP &= ~(1<<9);
    EE_WriteVariable(VirtAddVarTab[ERROR], TEMP); 
    error_stat &= ~(1<<9);
   }

    
 }
 
else if((TEMP & (1<<9)) !=0)
{
  TEMP &= ~(1<<9);
  EE_WriteVariable(VirtAddVarTab[ERROR], TEMP); 
  
}
  
}
/*******************************************************************************
********************************************************************************
**                                                                            **
**                        ����� ���������                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/