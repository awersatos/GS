/*******************************************************************************
********************************************************************************
**                                                                            **
**             БИБЛИОТЕКА ФУНКЦИЙ УПРАВЛЕНИЯ МОДЕМОМ WISMO 228                **
**                                                                            **
********************************************************************************
*******************************************************************************/


//****************Подключаемые файлы********************************************

#include "stm32f10x.h"
#include "main.h"
#include "GSM.h"
#include <string.h>
#include <stdio.h>

//************* Инициализация глобальных переменных ****************************
char IMEI[] = "123456789012345"; //Массив для IMEI
uint8_t OPERATOR; //Оператор сотовой связи
uint8_t SendDataError = 0;  //Ошибка передачи данных
//******************Статичные функции*******************************************
static char REG_NET(void);  //Проверка регистации в сети
static FunctionalState START_TCP_IP(void); //Функция запуска стека TCP/IP
//******************************************************************************

//==============================================================================
                  /*ФУНКЦИИ ДЛЯ РАБОТЫ С МОДЕМОМ*/
//==============================================================================
void GSM_Configuration(void) //Инициализация GSM
{
  FunctionalState stack = DISABLE; //Флаг открытия TCP/IP стека
  uint8_t i; //Счетчик
  char *im; //Ссылочная переменная для IMEI
  
do{
 do{
    if(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0) //Если модем выключен
    {
      delay_ms(200); 
      GPIO_ResetBits(GSM_MOD , GSM_ON); //Включаем модем
      delay_ms(800); 
      GPIO_SetBits(GSM_MOD , GSM_ON);
     }
 
    else //Если модем включен
   {
      GPIO_ResetBits(GSM_MOD , GSM_RESET); //Перезагружаем  модем
      delay_ms(100);
      GPIO_SetBits(GSM_MOD , GSM_RESET); //Устанавливакм бит сброса   
   }
    delay_ms(1500);
   }while(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0); //Ожидание готовности модема
 
 delay_ms(1000);
 //-----------------------------------------------------------------------------
  SendString_InUnit("AT+GSN\r\n" );  //Запрос IMEI  
  delay_ms(100);
  im=strstr(RxBuffer , "\r\n\r\n");
   if (im !=NULL)
   {
    im=im+4;
    for(i=0;i<15;i++) IMEI[i]=*im++; //Копирование IMEI в переменную
    Reset_rxDMA_ClearBufer(); //Сброс буфера
   }
 //-----------------------------------------------------------------------------
   
  SendString_InUnit("AT+CLIP=1\r\n"); //Включение определителя номера
  delay_ms(100);
  Reset_rxDMA_ClearBufer(); //Сброс буфера
  
  SendString_InUnit("AT+CPIN?\r\n"); //Проверка готовности SIM карты
  delay_ms(500);

  if (strstr(RxBuffer , "READY") !=NULL) //Если СИМ карта опознана
   {
     Reset_rxDMA_ClearBufer(); //Сброс буфера
     for(i=0;i<10;i++) //Проверка регистрации в сети
     {
      if(REG_NET()=='R') //Если регистрация успешна
      {
        
        SendString_InUnit("AT+COPS?\r\n"); //Определение оператора сотовой связи
        delay_ms(100);
        if (strstr(RxBuffer , "Beeline") !=NULL) OPERATOR=Beeline_OP;
        if (strstr(RxBuffer , "MTS") !=NULL) OPERATOR=MTS_OP;
        if (strstr(RxBuffer , "MegaFon") !=NULL) OPERATOR=Megafon_OP;
        Reset_rxDMA_ClearBufer(); //Сброс буфера
        
        for(i=0;i<3;i++) //Запуск TCP/IP стека
        {
         stack =  START_TCP_IP();  
         if(stack == ENABLE) break;
         delay_ms(3000); 
        }
        
        break;
      }     
     }   
    }
   }while(stack == DISABLE);
//------------------------------------------------------------------------------
  SendString_InUnit("AT+CMGF=1\r\n"); //Переключение в режим текстовых сообщений
  delay_ms(100); 
  Reset_rxDMA_ClearBufer(); //Сброс буфера

  SendDataError = 0;

  
}
//==============================================================================
static char REG_NET(void)  //Проверка регистации в сети
{ char *d;
  Reset_rxDMA_ClearBufer(); //Сброс буфера
  SendString_InUnit("AT+CREG?\r\n");
  delay_ms(100);
  d=strstr(RxBuffer , "+CREG:");
   if(d!=NULL)
   {
     d=d+9;
     if((*d=='1')|(*d=='5')) 
     { Reset_rxDMA_ClearBufer(); //Сброс буфера
       return 'R';}    
   }
    Reset_rxDMA_ClearBufer(); //Сброс буфера
   return NULL; 
}
//==============================================================================
static FunctionalState START_TCP_IP(void) //Функция запуска стека TCP/IP
{
  FunctionalState br = DISABLE;
  
SendString_InUnit("AT+WIPCFG=1\r\n"); //Запуск TCP/IP стека
  delay_ms(100);
   if(strstr(RxBuffer , "ERROR") !=NULL) //Если стек не запускается останавливаем его и запускаем снова
   {
     
    Reset_rxDMA_ClearBufer(); //Сброс буфера
    SendString_InUnit("AT+WIPCFG=0\r\n"); //Остановка TCP/IP стека
    delay_ms(300);
    Reset_rxDMA_ClearBufer(); //Сброс буфера
    SendString_InUnit("AT+WIPCFG=1\r\n"); //Запуск TCP/IP стека     
    delay_ms(300);
   }
  
  if(strstr(RxBuffer , "OK") !=NULL) //Если стек запустился
  {
   Reset_rxDMA_ClearBufer(); //Сброс буфера 
   SendString_InUnit("AT+WIPBR=1,6\r\n");  //Открытие GPRS барьера
   delay_ms(100);
//------------------------------------------------------------------------------   
   switch(OPERATOR) //Установка параметров GPRS барьера в зависимости от оператора
   {
   case Beeline_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.beeline.ru\"\r\n"); //Установка APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"beeline\"\r\n"); //Ввод логина 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"beeline\"\r\n"); //Ввод пароля
      delay_ms(100);
      break;
     }
     
   case MTS_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.mts.ru\"\r\n"); //Установка APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"mts\"\r\n"); //Ввод логина 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"mts\"\r\n"); //Ввод пароля
      delay_ms(100);
      break;
     }  
    case Megafon_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet\"\r\n"); //Установка APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"\"\r\n"); //Ввод логина 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"\"\r\n"); //Ввод пароля
      delay_ms(100);
      break;
     }   
   }
//------------------------------------------------------------------------------   
    Reset_rxDMA_ClearBufer(); //Сброс буфера
   
    SendString_InUnit("AT+WIPBR=4,6,0\r\n"); //Запуск барьера
    for (uint8_t cntr=0;cntr<50;cntr++)
    {
      if(strstr(RxBuffer , "OK") !=NULL)
      {
       br = ENABLE;
       break;
      }
      if(strstr(RxBuffer , "ERROR") !=NULL)
      {
       br = DISABLE;
       break;
      }  
      delay_ms(1000);

      br = DISABLE;
    }
  }
  Reset_rxDMA_ClearBufer(); //Сброс буфера  
  
  return br;
  
}
//==============================================================================


/*******************************************************************************
********************************************************************************
**                                                                            **
**                        КОНЕЦ ПРОГРАММЫ                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/