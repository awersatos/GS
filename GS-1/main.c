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
#include <string.h>
//************* ������������� ���������� ���������� ****************************
uint32_t delay_Counter; //�������� ��������� ��������

char TxBuffer[BufferSize+1]; //���������� Bluetooth ����� USART1
char RxBuffer[BufferSize+1]; //�������� Bluetooth ����� USART1


uint16_t adc_buffer[11-1]; //
uint16_t typ_pin[13-1]; //0-��������� >1-������

uint16_t max[8-1], min[8-1], timmin[8-1];
uint8_t count, index, state;
uint32_t error = 0; //������ �� �������� ����������� ������� � �������
uint32_t temperlim = 100;

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
GPIO_Configuration(); //������������� ������ ����� ������
EXTI_Configuration(); //������������� ����������� ������� ����������
NVIC_Configuration();//������������� ����������
ADC_Configuration();//������������� ���
DMA_Configuration(); //������������� ������� DMA
TIMER_Configuration();//������������� ��������
UART_Configuration();//������������� USART
SysTick_Config(SystemCoreClock / 1000); //���������� ���������� ������� 1��
NVIC_SetPriority(SysTick_IRQn,0); //������������������� ���������� ������� ���������



index=0;
count=0;
state=0;
max[0]=adc_buffer[6]; //PB0
min[0]=adc_buffer[6]; //PB0
timmin[0]=0;

  while(1)//�������� ���� ���������
  {
   //if (typ_pin[6] == 1) Temperature(adc_buffer[6]);
   //if (typ_pin[7] == 1) Temperature(adc_buffer[7]);
   Dathiki();
   //if (error > 0) STM32vldiscovery_LEDOn(LED4); 
  }//����� ��������� �����
}
//==============================================================================
             /*����� �������� ������� ���������*/
//==============================================================================
static void RCC_Configuration(void) //��������� ����������� � ��������� ������������� ���������
{
 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);  
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
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

     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //��������� ������� 220�
     GPIO_Init(GPIOC, &GPIO_InitStructure);


     // ����������� ���� PA10 ��� ���� UART� (RxD)
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
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
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
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
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
     GPIO_Init(GPIOC, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
     GPIO_Init(GPIOC, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
     GPIO_Init(GPIOC, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
     GPIO_Init(GPIOC, &GPIO_InitStructure); 
     
     
     GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
     GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
  
}
//==============================================================================
static void EXTI_Configuration(void) //������������� ����������� ������� ����������
{
  EXTI_InitTypeDef EXTI_InitStructure;
  
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
}
//==============================================================================
static void NVIC_Configuration(void)//������������� ����������
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//����������� ����� �����������:2 ���������
  NVIC_InitTypeDef NVIC_InitStruct; //��������� ��������� ��������� ����������
  
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
  
  NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
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
     TIM3->DIER |= TIM_DIER_UIE; //��������� ���������� �� �������
     
     TIM_Cmd(TIM3, ENABLE);// ������ �������

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
void Dathiki(void)
{

   if (typ_pin[0] == 1) {
     if (adc_buffer[0] < 1000) error|= (1 << 4);
   }
   if (typ_pin[1] == 1) {
     if (adc_buffer[1] < 1000) error|= (1 << 5);
   }
   if (typ_pin[2] == 1) {
     if (adc_buffer[2] < 1000) error|= (1 << 6);
   }
   if (typ_pin[3] == 1) {
     if (adc_buffer[3] < 1000) error|= (1 << 7);
   }
   if (typ_pin[4] == 1) {
     if (adc_buffer[4] < 1000) error|= (1 << 8);
   }
   if (typ_pin[5] == 1) {
     if (adc_buffer[5] < 1000) error|= (1 << 9);
   }
   if (typ_pin[6] == 1) {
     if (adc_buffer[6] < 1000) error|= (1 << 10);
   }
   if (typ_pin[7] == 1) {
     if (adc_buffer[7] < 1000) error|= (1 << 11);
   }
} 
//==============================================================================
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
/*******************************************************************************
********************************************************************************
**                                                                            **
**                        ����� ���������                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/