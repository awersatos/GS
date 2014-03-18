/*******************************************************************************
********************************************************************************
**                                                                            **
**             ���������� ������� ���������� ������� WISMO 228                **
**                                                                            **
********************************************************************************
*******************************************************************************/


//****************������������ �����********************************************

#include "stm32f10x.h"
#include "main.h"
#include "GSM.h"
#include <string.h>
#include <stdio.h>

//************* ������������� ���������� ���������� ****************************
char IMEI[] = "123456789012345"; //������ ��� IMEI
uint8_t OPERATOR; //�������� ������� �����
uint8_t SendDataError = 0;  //������ �������� ������
//******************��������� �������*******************************************
static char REG_NET(void);  //�������� ���������� � ����
static FunctionalState START_TCP_IP(void); //������� ������� ����� TCP/IP
//******************************************************************************

//==============================================================================
                  /*������� ��� ������ � �������*/
//==============================================================================
void GSM_Configuration(void) //������������� GSM
{
  FunctionalState stack = DISABLE; //���� �������� TCP/IP �����
  uint8_t i; //�������
  char *im; //��������� ���������� ��� IMEI
  
do{
 do{
    if(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0) //���� ����� ��������
    {
      delay_ms(200); 
      GPIO_ResetBits(GSM_MOD , GSM_ON); //�������� �����
      delay_ms(800); 
      GPIO_SetBits(GSM_MOD , GSM_ON);
     }
 
    else //���� ����� �������
   {
      GPIO_ResetBits(GSM_MOD , GSM_RESET); //�������������  �����
      delay_ms(100);
      GPIO_SetBits(GSM_MOD , GSM_RESET); //������������� ��� ������   
   }
    delay_ms(1500);
   }while(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0); //�������� ���������� ������
 
 delay_ms(1000);
 //-----------------------------------------------------------------------------
  SendString_InUnit("AT+GSN\r\n" );  //������ IMEI  
  delay_ms(100);
  im=strstr(RxBuffer , "\r\n\r\n");
   if (im !=NULL)
   {
    im=im+4;
    for(i=0;i<15;i++) IMEI[i]=*im++; //����������� IMEI � ����������
    Reset_rxDMA_ClearBufer(); //����� ������
   }
 //-----------------------------------------------------------------------------
   
  SendString_InUnit("AT+CLIP=1\r\n"); //��������� ������������ ������
  delay_ms(100);
  Reset_rxDMA_ClearBufer(); //����� ������
  
  SendString_InUnit("AT+CPIN?\r\n"); //�������� ���������� SIM �����
  delay_ms(500);

  if (strstr(RxBuffer , "READY") !=NULL) //���� ��� ����� ��������
   {
     Reset_rxDMA_ClearBufer(); //����� ������
     for(i=0;i<10;i++) //�������� ����������� � ����
     {
      if(REG_NET()=='R') //���� ����������� �������
      {
        
        SendString_InUnit("AT+COPS?\r\n"); //����������� ��������� ������� �����
        delay_ms(100);
        if (strstr(RxBuffer , "Beeline") !=NULL) OPERATOR=Beeline_OP;
        if (strstr(RxBuffer , "MTS") !=NULL) OPERATOR=MTS_OP;
        if (strstr(RxBuffer , "MegaFon") !=NULL) OPERATOR=Megafon_OP;
        Reset_rxDMA_ClearBufer(); //����� ������
        
        for(i=0;i<3;i++) //������ TCP/IP �����
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
  SendString_InUnit("AT+CMGF=1\r\n"); //������������ � ����� ��������� ���������
  delay_ms(100); 
  Reset_rxDMA_ClearBufer(); //����� ������

  SendDataError = 0;

  
}
//==============================================================================
static char REG_NET(void)  //�������� ���������� � ����
{ char *d;
  Reset_rxDMA_ClearBufer(); //����� ������
  SendString_InUnit("AT+CREG?\r\n");
  delay_ms(100);
  d=strstr(RxBuffer , "+CREG:");
   if(d!=NULL)
   {
     d=d+9;
     if((*d=='1')|(*d=='5')) 
     { Reset_rxDMA_ClearBufer(); //����� ������
       return 'R';}    
   }
    Reset_rxDMA_ClearBufer(); //����� ������
   return NULL; 
}
//==============================================================================
static FunctionalState START_TCP_IP(void) //������� ������� ����� TCP/IP
{
  FunctionalState br = DISABLE;
  
SendString_InUnit("AT+WIPCFG=1\r\n"); //������ TCP/IP �����
  delay_ms(100);
   if(strstr(RxBuffer , "ERROR") !=NULL) //���� ���� �� ����������� ������������� ��� � ��������� �����
   {
     
    Reset_rxDMA_ClearBufer(); //����� ������
    SendString_InUnit("AT+WIPCFG=0\r\n"); //��������� TCP/IP �����
    delay_ms(300);
    Reset_rxDMA_ClearBufer(); //����� ������
    SendString_InUnit("AT+WIPCFG=1\r\n"); //������ TCP/IP �����     
    delay_ms(300);
   }
  
  if(strstr(RxBuffer , "OK") !=NULL) //���� ���� ����������
  {
   Reset_rxDMA_ClearBufer(); //����� ������ 
   SendString_InUnit("AT+WIPBR=1,6\r\n");  //�������� GPRS �������
   delay_ms(100);
//------------------------------------------------------------------------------   
   switch(OPERATOR) //��������� ���������� GPRS ������� � ����������� �� ���������
   {
   case Beeline_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.beeline.ru\"\r\n"); //��������� APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"beeline\"\r\n"); //���� ������ 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"beeline\"\r\n"); //���� ������
      delay_ms(100);
      break;
     }
     
   case MTS_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.mts.ru\"\r\n"); //��������� APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"mts\"\r\n"); //���� ������ 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"mts\"\r\n"); //���� ������
      delay_ms(100);
      break;
     }  
    case Megafon_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet\"\r\n"); //��������� APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"\"\r\n"); //���� ������ 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"\"\r\n"); //���� ������
      delay_ms(100);
      break;
     }   
   }
//------------------------------------------------------------------------------   
    Reset_rxDMA_ClearBufer(); //����� ������
   
    SendString_InUnit("AT+WIPBR=4,6,0\r\n"); //������ �������
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
  Reset_rxDMA_ClearBufer(); //����� ������  
  
  return br;
  
}
//==============================================================================


/*******************************************************************************
********************************************************************************
**                                                                            **
**                        ����� ���������                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/