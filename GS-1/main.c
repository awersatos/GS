/*******************************************************************************
********************************************************************************
**                                                                            **
**                  ОСНОВНОЙ ФАЙЛ ПРОЕКТА GS-1                                **
**                                                                            **
********************************************************************************
*******************************************************************************/


//****************Подключаемые файлы********************************************
#include "stm32f10x_conf.h"
#include "eeprom.h"
#include "stm32f10x.h"
#include "main.h"
#include <string.h>
//************* Инициализация глобальных переменных ****************************
uint32_t delay_Counter; //Интервал программы задержки

char TxBuffer[BufferSize+1]; //Передающий Bluetooth буфер USART1
char RxBuffer[BufferSize+1]; //Приемный Bluetooth буфер USART1


uint16_t adc_buffer[11-1]; //
uint16_t typ_pin[13-1]; //0-выключено >1-режимы

uint16_t max[8-1], min[8-1], timmin[8-1];
uint8_t count, index, state;
uint32_t error = 0; //ошибка от датчиков температуры шлейфов и питания
uint32_t temperlim = 100;

//Массив виртуальных адресов EEPROM
const uint16_t VirtAddVarTab[NumbOfVar] = 
{ 0xAB00, 0xAB01, 0xAB02, 0xAB03, 0xAB04, 0xAB05,0xAB06, 0xAB07, 0xAB08, 0xAB09, 0xAB0A, 0xAB0B, 0xAB0C, 0xAB0D, 0xAB0E, 0xAB0F,
  
  0xAB10, 0xAB11, 0xAB12, 0xAB13, 0xAB14, 0xAB15,0xAB16, 0xAB17, 0xAB18, 0xAB19, 0xAB1A, 0xAB1B, 0xAB1C, 0xAB1D, 0xAB1E, 0xAB1F,
  
  0xAB20, 0xAB21, 0xAB22, 0xAB23, 0xAB24, 0xAB25,0xAB26, 0xAB27, 0xAB28, 0xAB29, 0xAB2A, 0xAB2B, 0xAB2C, 0xAB2D, 0xAB2E, 0xAB2F,
    
  0xAB30, 0xAB31, 0xAB32, 0xAB33, 0xAB34, 0xAB35,0xAB36, 0xAB37, 0xAB38, 0xAB39, 0xAB3A, 0xAB3B, 0xAB3C, 0xAB3D, 0xAB3E, 0xAB3F
};


//******************Статичные функции*******************************************
static void RCC_Configuration(void); //Включение тактироания и настройка перриферийных устройств
static void GPIO_Configuration(void); //Инициализация портов ввода вывода
static void EXTI_Configuration(void); //Инициализация контроллера внешних прерываний
static void NVIC_Configuration(void);//Инициализация прерываний
static void ADC_Configuration(void);//Инициализация АЦП
static void DMA_Configuration(void); //Инициализация каналов DMA
static void TIMER_Configuration(void);//Инициализация таймеров
static void UART_Configuration(void);    //Инициализация USART
static void ClearBufer(char *buf); //Функция очистки буфера
//******************************************************************************


//==============================================================================
                  /*НАЧАЛО ИСПОЛНЯЕМОГО КОДА ПРОГРАММЫ*/
//==============================================================================


void main() //Основная фукция программы
{
          /*ИНИЦИАЛИЗАЦИЯ ПЕРЕФЕРИЙНЫХ УСТРОЙСТВ КОНТРОЛЛЕРА*/  
FLASH_Unlock(); //Разблокировка контроллера флеш памяти
EE_Init(); //Инициализация виртуального EEPROM
RCC_Configuration(); //Включение тактироания и настройка перриферийных устройств
GPIO_Configuration(); //Инициализация портов ввода вывода
EXTI_Configuration(); //Инициализация контроллера внешних прерываний
NVIC_Configuration();//Инициализация прерываний
ADC_Configuration();//Инициализация АЦП
DMA_Configuration(); //Инициализация каналов DMA
TIMER_Configuration();//Инициализация таймеров
UART_Configuration();//Инициализация USART
SysTick_Config(SystemCoreClock / 1000); //Прерывание системного таймера 1мС
NVIC_SetPriority(SysTick_IRQn,0); //Приоритетпрерывания системного таймера наивысший



index=0;
count=0;
state=0;
max[0]=adc_buffer[6]; //PB0
min[0]=adc_buffer[6]; //PB0
timmin[0]=0;

  while(1)//Основной цикл программы
  {
   //if (typ_pin[6] == 1) Temperature(adc_buffer[6]);
   //if (typ_pin[7] == 1) Temperature(adc_buffer[7]);
   Dathiki();
   //if (error > 0) STM32vldiscovery_LEDOn(LED4); 
  }//Конец основного цикла
}
//==============================================================================
             /*КОНЕЦ ОСНОВНОЙ ФУНКЦИИ ПРОГРАММЫ*/
//==============================================================================
static void RCC_Configuration(void) //Включение тактироания и настройка перриферийных устройств
{
 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);  
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
}
//==============================================================================
static void GPIO_Configuration(void) //Инициализация портов ввода вывода
{
 GPIO_InitTypeDef GPIO_InitStructure;

//ADC_Channel_0-PA0, 1-A1, 2-A2, 3-A3, 4-A4, 5-A5, 6-A6,7-A7, 8-B0, 9-B1, 10-C0, 11-C1, 12-C2, 13-C3, 14-C4, 15-C5
// Настраиваем пин на работу в режиме аналогового входа
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //только датчики
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //только датчики
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //только датчики
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //только датчики
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //только датчики
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //только датчики
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //датчики и режим телеметрии
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //датчики и режим газоанализатора
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //Температура
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //Температура
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //Питание 12В
     GPIO_Init(GPIOC, &GPIO_InitStructure);

     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //состояние питания 220в
     GPIO_Init(GPIOC, &GPIO_InitStructure);


     // Настраиваем ногу PA10 как вход UARTа (RxD)
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
     GPIO_Init(GPIOA, &GPIO_InitStructure);
     
     // Настраиваем ногу PA9 как выход UARTа (TxD)
     // Причем не просто выход, а выход с альтернативной функцией
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


     //PB5 PB6 прерывание на 220/40 и 12В
     GPIO_StructInit(&GPIO_InitStructure);
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
     GPIO_Init(GPIOB, &GPIO_InitStructure);

     //PC2 PC3 PC4 PC5 светодиоды
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
static void EXTI_Configuration(void) //Инициализация контроллера внешних прерываний
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
static void NVIC_Configuration(void)//Инициализация прерываний
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//Кофигурация групп приоритетов:2 подгруппы
  NVIC_InitTypeDef NVIC_InitStruct; //Объявляем структуру настройки прерываний
  
  NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn; //Прерывания внешние линии 5-9
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //Приоритетная подгруппа
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //Приоритет
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
  NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию
  
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
static void ADC_Configuration(void)//Инициализация АЦП
{
     ADC_InitTypeDef ADC_InitStructure;

     ADC_StructInit(&ADC_InitStructure);
    
     ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //Режим – от триггера(внешнего события), регулярный
     ADC_InitStructure.ADC_ScanConvMode = ENABLE;//DISABLE;
     ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //без продолжения
     ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//T3_TRGO;
     ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
     ADC_InitStructure.ADC_NbrOfChannel = 11;
     ADC_Init(ADC1, &ADC_InitStructure);
  
     // Включаем каналы первого модуля АЦП
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
     
     // включаем АЦП
     ADC_DMACmd(ADC1, ENABLE);
     ADC_Cmd(ADC1, ENABLE);
     
     // Калибровка АЦП
     ADC_ResetCalibration(ADC1); //сброс калибровки
     while (ADC_GetResetCalibrationStatus(ADC1));
     ADC_StartCalibration(ADC1);
     while (ADC_GetCalibrationStatus(ADC1));
     ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
  
  
}
//==============================================================================
static void DMA_Configuration(void) //Инициализация каналов DMA
{
     DMA_InitTypeDef DMA_InitStructure;
  
     DMA_StructInit(&DMA_InitStructure);
  
     DMA_DeInit(DMA1_Channel1);     // Данные будем брать из регистра данных ADC1
     DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
     DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_buffer; // Переправлять данные будем в переменную
     DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  // Передача данных из периферии в память
     DMA_InitStructure.DMA_BufferSize = 11; // Размер буфера
     DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// Адрес источника данных не инкрементируем
     DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;// Память инкрементировать
     DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // Настройки размера данных//16бит
     DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
     DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;    //DMA_Mode_Normal;
     DMA_InitStructure.DMA_Priority = DMA_Priority_High;
     DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
     DMA_Init(DMA1_Channel1, &DMA_InitStructure);
     
       //Прерывание по завершению и полузавершении
     DMA_ITConfig(DMA1_Channel1, DMA_IT_TC /*| DMA_IT_HT*/, ENABLE);
         
     DMA_Cmd(DMA1_Channel1, ENABLE);// Включаем первый канал DMA1
     
     /*USART1_TX-Канал 4*/
  DMA_DeInit(DMA1_Channel4);  //Сброс настроек канала 4
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //Базовый адрес регистра данных USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBuffer; //Передающий буфер USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //Направление данных из памяти к перефирии
  DMA_InitStructure.DMA_BufferSize = BufferSize; //Размер буфера
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //Инкремент перефирийного регистра запрещен
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //Инкремент памяти разрешен 
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //Размер данных в перефирии 1 байт
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //Размер данных в памяти 1 байт
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //Нормальный режим работы DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //Приоритет  низкий
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //Передача из памяти в память выключена
  
  DMA_Init(DMA1_Channel4 , &DMA_InitStructure); //Передаем структуру в функцию
 // DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE); //Разрешаем прерывание по окончании передачи
  
  /*USART1_RX-Канал5*/
  DMA_DeInit(DMA1_Channel5);  //Сброс настроек канала 5
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //Базовый адрес регистра данных USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBuffer; //Приемный буфер USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //Направление данных от периферии к памяти
  DMA_InitStructure.DMA_BufferSize = BufferSize; //Размер буфера
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //Циклический режим работы DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //Приоритет  высокий
  
  DMA_Init(DMA1_Channel5 , &DMA_InitStructure); //Передаем структуру в функцию

}
//==============================================================================
static void TIMER_Configuration(void)//Инициализация таймеров
{
     TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   
     TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
       //24000000 частота 72000000
     TIM_TimeBaseStructure.TIM_Period = 2000;       //таймер тикает 1000 раз в секунду (если частота 24000000Гц)
     TIM_TimeBaseStructure.TIM_Prescaler = 36000 - 1;
     TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
     TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
     TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 
     
     TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update); // выход синхронизации
     TIM3->DIER |= TIM_DIER_UIE; //разрешаем прерывание от таймера
     
     TIM_Cmd(TIM3, ENABLE);// запуск таймера

}
//==============================================================================
static void UART_Configuration(void)    //Инициализация USART
{
 USART_InitTypeDef USART_InitStructure; //Объявление структуры инциализации
 USART_StructInit(&USART_InitStructure); //Очистка полей структуры
                   /*USART1*/
  USART_InitStructure.USART_BaudRate = 115200;  //Скорость 115200 б/с
  USART_InitStructure.USART_WordLength = USART_WordLength_8b; //Биты данных 8
  USART_InitStructure.USART_StopBits = USART_StopBits_1;  //Стоповые 1
  USART_InitStructure.USART_Parity = USART_Parity_No;     //Четность нет
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Управляющие сигналы нет
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //Передатчик и приемник включены
  
  USART_Init(USART1, &USART_InitStructure); //Передаем структуру в функцию
  
  USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //Разрешить запрос DMA на прием и передачу
  USART_Cmd(USART1, ENABLE); //Активировать USART1
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
void Temperature(uint16_t temp) //функция сигнализации минимальной температуры
{
if (temp > max[index]) max[index]=temp; //наростает максимум текущего цикла
if (temp < min[index]) min[index]=temp; //наростает минимум текущего цикла
if (count > 0) //если у нас уже прошел первый цикл
   {
     if (max[(index-1)&3]-min[(index-1)&3]*0.15 <= min[(index-1)&3]-min[index]) error |= 1; //ОШИБКА температура опустилась и котёл перестал работать
     if ((state == 1) & (temp <= (min[(index-1)&3]+((max[(index-1)&3]-min[(index-1)&3])*0.3)))) state = 1; //знаем max и почти дошли до min
     if ((state == 0) & (temp >= (min[(index-1)&3]+((max[(index-1)&3]-min[(index-1)&3])*0.7)))) state = 2; //знаем max и min почти дошли до max
     if (state == 2)
     {
       if (count < 7) count++;
       index= (index+1)&3; //цикл закончен добавляем 1 и обнуляем все биты кроме 2, т.к. буфер до 8 и он кольцевой
       state = 0;
       max[index]=temp;
       min[index]=temp;
     }
   }
else //если у нас ещё не прошел первый цикл
   {
     if (max[index] - min[index] > temperlim) 
                //если котёл работает
     {
       if ((state == 1) & (temp <= (max[index]+min[index])*0.5)) state = 3; //после подъёма до max она падает и проходит середину
       if ((state == 2) & (temp >= (max[index]+min[index])*0.5)) state = 4; //после опускания до min она поднимается и проходит середину
       if ((state == 3) & (temp >= (min[index]+((max[index]-min[index])*0.7)))) state = 5; //теперь знаем min и почти дошли до max
       if ((state == 4) & (temp <= (min[index]+((max[index]-min[index])*0.3)))) state = 6; //теперь знаем max и почти дошли до min
       if ((state == 5) & (temp <= (min[index]+((max[index]-min[index])*0.3)))) state = 6; //почти дошли до min
       if (state == 6)
       {
         count++; //уходим на заполнение след цикла и обработку сигнализации минимальной температуры
         index++;
         state = 0;
         max[index]=temp;
         min[index]=temp;
       }
     }
   }
}
//==============================================================================
void delay_ms(uint16_t msec) //Задержка в милисекундах
{
 delay_Counter = msec;
 while(delay_Counter != 0);
  
}
//==============================================================================
void SendString_InUnit(const char *str) //Функция отправки строки навнешний модуль через UART
{
 uint16_t len; //Длинна строки
 uint16_t i;   //Счетчик
 len=strlen(str); //Вычисляем длинну строки
 
 for(i=0;i<len;i++)  TxBuffer[i]=*str++; //Записывем данные в буфер передачи
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //Отключаем канал DMA
  DMA_SetCurrDataCounter(DMA1_Channel4 , len);      //Устанавливаем счетчик DMA канала
  DMA_ClearFlag(DMA1_FLAG_TC4);                     //Сбрасываем флаг окончания передачи
  DMA_Cmd(DMA1_Channel4 , ENABLE );                 //Активируем DMA канал
  while(DMA_GetFlagStatus(DMA1_FLAG_TC4)==RESET);   //Ждем окончания передачи
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //Отключаем канал DMA
  ClearBufer(TxBuffer);                   //Очищаем буфер 
}
//==============================================================================
static void ClearBufer(char *buf) //Функция очистки буфера
{
 uint16_t j; //Счетчик 
 for(j=0;j<BufferSize;j++) 
 {
   *buf=0x00; //Присваиваем текущему элементу значение по умолчание
   buf++; //Инкрементируем ссылочную переменную
 }
}
//==============================================================================
void Reset_rxDMA_ClearBufer(void) //Сброс приемного DMA канала и очистка буфера приема
{
     DMA_Cmd(DMA1_Channel5 , DISABLE ); //Останавливаем DMA канал
     ClearBufer(RxBuffer);                  //Очищаем буфер
     DMA_SetCurrDataCounter(DMA1_Channel5 , BufferSize); //Ставим счетчик в размер буфера
     DMA_Cmd(DMA1_Channel5 , ENABLE ); //Активируем DMA канал
 
  
}
/*******************************************************************************
********************************************************************************
**                                                                            **
**                        КОНЕЦ ПРОГРАММЫ                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/