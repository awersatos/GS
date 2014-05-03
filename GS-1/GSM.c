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
#include <ctype.h>

//************* ������������� ���������� ���������� ****************************
const char SERIAL_NUMBER[] = "1231231230" ; //�������� ����� ����������

char IMEI[] = "123456789012345"; //������ ��� IMEI
const char SERVER[]= "\"gs.ggsbarnaul.ru\""; //������
const char PORT[]= ",80\r\n"; //����

uint8_t OPERATOR; //�������� ������� �����
uint8_t SendDataError = 0;  //������ �������� ������
//******************��������� �������*******************************************
static char REG_NET(void);  //�������� ���������� � ����
static FunctionalState START_TCP_IP(void); //������� ������� ����� TCP/IP
static void READ_PHONE_EEPROM(char *ph, uint8_t num );//���������� ����������� ������ �� EEPROM
static void SMS_Command_PARSER(uint8_t num); //������ ��� ������
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
      delay_ms(1000); 
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
      delay_ms(1000);  
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
void SEND_SMS(SMS_TypeDef sms, uint8_t number) //������� �������� ���
{
 uint8_t i, j; //������� 
 FunctionalState Send = DISABLE; //������ �������� ���������
 char PHONE_NUMBER[] = "123456789012"; //������ ������
 char PDU_PHONE_NUMBER[] = "123456789012"; //������ ������ � PDU �������
 
   READ_PHONE_EEPROM(PHONE_NUMBER, number); //���������� ������ �� EEPROM

 for(i=0; i<10; i+=2) PDU_PHONE_NUMBER[i] = PHONE_NUMBER[i+2]; //���������� ������� PDU
 PDU_PHONE_NUMBER[10] = 'F';
 for(i=1; i<12; i+=2) PDU_PHONE_NUMBER[i] = PHONE_NUMBER[i];
 Reset_rxDMA_ClearBufer(); //����� ������
 delay_ms(500);
 SendString_InUnit("AT+CMGF=0\r\n"); //��������� PDU ������
 delay_ms(100);
 
for(j=0;j<5;j++) //������� �������� ���������
 {
 
 SendString_InUnit("AT+CMGS="); //������� �������� ���������
 
 switch(sms) //����� ������ ���������
 {
//------------------------------------------------------------------------------   
 case NUMBER_SET:
   SendString_InUnit("37\r\n");
   break;
//------------------------------------------------------------------------------
 case RESET_OK:
   SendString_InUnit("57\r\n");
   break;
//------------------------------------------------------------------------------
 case COMMAND_COMPLETE:
   SendString_InUnit("47\r\n");
   break;
//------------------------------------------------------------------------------
 case COMMAND_ERROR:
   SendString_InUnit("49\r\n");
   break;
//------------------------------------------------------------------------------
 case GAS_ALARM:
   SendString_InUnit("51\r\n");
   break;
//------------------------------------------------------------------------------
 case TEMP_ALARM:
   SendString_InUnit("79\r\n");
   break;
//------------------------------------------------------------------------------
 case SMOK:
   SendString_InUnit("51\r\n");
   break;
//------------------------------------------------------------------------------
 case FIRE:
   SendString_InUnit("45\r\n");
   break;
//------------------------------------------------------------------------------ 
 case VOLTAGE_MISSING:
   SendString_InUnit("61\r\n");
   break;
//------------------------------------------------------------------------------
 case VOLTAGE_MISSING_3H:
   SendString_InUnit("93\r\n");
   break;
//------------------------------------------------------------------------------ 
 case VOLTAGE_ON:
   SendString_InUnit("67\r\n");
   break;
//------------------------------------------------------------------------------  
 case LOW_BAT:
   SendString_InUnit("65\r\n");
   break;
//------------------------------------------------------------------------------
 case INVASION:
   SendString_InUnit("47\r\n");
   break;
//------------------------------------------------------------------------------ 
case OPTIONS:
   EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP); 
   if((TEMP & (1<<2)) != 0) SendString_InUnit("141\r\n");
   else SendString_InUnit("109\r\n");
   break;
//------------------------------------------------------------------------------ 
 case COTEL_BLOK:
   SendString_InUnit("49\r\n");
   break;
//------------------------------------------------------------------------------ 
  case HOME_TEMP:
   SendString_InUnit("77\r\n");
   break;
//------------------------------------------------------------------------------  
   
 }

 delay_ms(100);
 
 if(strchr(RxBuffer , '>')!=NULL)
 {
 Reset_rxDMA_ClearBufer(); //����� ������
 
 SendString_InUnit("0001000B91"); //��������� PDU
 SendString_InUnit(PDU_PHONE_NUMBER); //����� � ������� PDU
 SendString_InUnit("0008"); //���������
 
 switch(sms) //���� ������ ���������
  {
//------------------------------------------------------------------------------    
   case NUMBER_SET:
    SendString_InUnit("18041D043E043C043504400020043F04400438043D044F0442");
    break;
//------------------------------------------------------------------------------    
   case RESET_OK:
    SendString_InUnit("2C0412044104350020043D0430044104420440043E0439043A04380020044104310440043E04480435043D044B");
    break;
//------------------------------------------------------------------------------ 
 case COMMAND_COMPLETE:
   SendString_InUnit("22041A043E043C0430043D04340430002004380441043F043E043B043D0435043D0430");
   break;
//------------------------------------------------------------------------------
 case COMMAND_ERROR:
   SendString_InUnit("24041E044804380431043A043000200421041C04210020043A043E043C0430043D0434044B");
   break;
//------------------------------------------------------------------------------ 
   case GAS_ALARM:
   SendString_InUnit("2604140430044204470438043A0020043304300437043000200442044004350432043E04330430");
   break;
//------------------------------------------------------------------------------
    case TEMP_ALARM:
   SendString_InUnit("42041A0440043804420438044704350441043A043E04350020043F043E043D043804360435043D04380435002004420435043C043F043504400430044204430440044B");
   break;
//------------------------------------------------------------------------------
 case SMOK:
   SendString_InUnit("2604140430044204470438043A00200434044B043C043000200442044004350432043E04330430");
   break;
//------------------------------------------------------------------------------ 
  case FIRE:
   SendString_InUnit("20041F043E043604300440043D0430044F00200442044004350432043E04330430");
   break;
//------------------------------------------------------------------------------ 
 case VOLTAGE_MISSING:
   SendString_InUnit("30041E0442043A043B044E04470435043D043E0020044D043B0435043A04420440043E043F043804420430043D04380435");
   break;
//------------------------------------------------------------------------------ 
 case VOLTAGE_MISSING_3H:
   SendString_InUnit("50042D043B0435043A04420440043E043F043804420430043D043804350020043E044204410443044204410442043204430435044200200431043E043B04350435002000330020044704300441043E0432");
   break;
//------------------------------------------------------------------------------ 
 case VOLTAGE_ON:
   SendString_InUnit("36042D043B0435043A04420440043E043F043804420430043D0438043500200432043E044104420430043D043E0432043B0435043D043E");
   break;
//------------------------------------------------------------------------------ 
 case LOW_BAT:
   SendString_InUnit("34041A0440043804420438044704350441043A0438043900200440043004370440044F043400200431043004420430044004350438");
   break;
//------------------------------------------------------------------------------ 
 case INVASION:
   SendString_InUnit("220422044004350432043E04330430002004320442043E044004360435043D04380435");
   break;
//------------------------------------------------------------------------------ 
 case COTEL_BLOK:
   SendString_InUnit("24041A043E04420435043B0020043704300431043B043E043A04380440043E04320430043D");
   break;
//------------------------------------------------------------------------------ 
  case HOME_TEMP:
   SendString_InUnit("400421043D043804360435043D04380435002004420435043C043F043504400430044204430440044B002004320020043F043E043C043504490435043D04380438");
   break;
//------------------------------------------------------------------------------   
 case OPTIONS:
   if((TEMP & (1<<2)) != 0) SendString_InUnit("80");  
   else SendString_InUnit("60");
   
   SendString_InUnit("042104420430044204430441003A0020"); //������: =8
   
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
   if((TEMP & 0x3FFF) ==0) SendString_InUnit("043D043E0440043C0430002C002000200020"); //�����,   =9
   else SendString_InUnit("0442044004350432043E04330430002C0020");  //�������, =9
   
   SendString_InUnit("04420435043C043F0435044004300442044304400430002004420435043F043B043E043D043E0441043804420435043B044F003A0020"); //����������� �������������: =27
   
   uint8_t t, t_h, t_l;
   char str_tmp[] = "0030";   
   t = TEMP_InGrad(1);
   if(t>99)  SendString_InUnit("00300030"); // 00 =2 ������ �����������
   else
   {
     t_h = t/10;
     t_l = t%10;
     str_tmp[3] = t_h + 0x30;
     SendString_InUnit(str_tmp); //������� �����
     str_tmp[3] = t_l + 0x30;
     SendString_InUnit(str_tmp);// ������� �����
   }
   SendString_InUnit("00B00421");//�� = 2
 //........  
   EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP); 
   if((TEMP & (1<<2)) != 0)
   {
    SendString_InUnit("002C0020043F043E043C043504490435043D0438044F003A"); //, ���������: =12
    t = TEMP_InGrad(2);
   if(t>99)  SendString_InUnit("00300030"); // 00 =2 ������ �����������
   else
   {
     t_h = t/10;
     t_l = t%10;
     str_tmp[3] = t_h + 0x30;
     SendString_InUnit(str_tmp); //������� �����
     str_tmp[3] = t_l + 0x30;
     SendString_InUnit(str_tmp);// ������� �����
   }
   SendString_InUnit("00B00421");//�� = 2
     
   }
   
   
   break;
//------------------------------------------------------------------------------
   
   
  }
  delay_ms(50);
 SendString_InUnit("\x1A"); //������� �������� ���������
 
 for( i=0; i<100; i++)
 { 
 delay_ms(50);
 if(strstr(RxBuffer , "OK") != NULL) 
  {
   Send = ENABLE;
   break;
  }
 delay_ms(50);
 if(strstr(RxBuffer , "ERROR") != NULL) break;
 } 
 }
Reset_rxDMA_ClearBufer(); //����� ������
if(Send == ENABLE) break;
delay_ms(2000);
} //����� �������

  SendString_InUnit("AT+CMGF=1\r\n"); //������������ � ����� ��������� ���������
  delay_ms(100);
  Reset_rxDMA_ClearBufer(); //����� ������


}
//==============================================================================
void RECEIVE_SMS(void) //������� ��������� ��� ���������
{
  
   char *start;
   char  *end; 
   char *data;
   uint8_t nbr = 0;
   char PHONE_NUMBER[] = "123456789012"; //������ ������
  
   Reset_rxDMA_ClearBufer(); //����� ������
   SendString_InUnit("AT+CMGL=\"ALL\"\r\n");  //������� ������ ���� ���������
   delay_ms(100);
   start = strstr(RxBuffer , "+CMGL: "); //����������� ������� ������ ���������
   

if(start!=NULL)
{
  EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); 
  TEMP &= (1<<15);
  if(TEMP !=0)
  {
  READ_PHONE_EEPROM(PHONE_NUMBER, PHONE1); //���������� ������ �� EEPROM
  end = strstr(start , "\r\n");
  data = strstr(RxBuffer , PHONE_NUMBER); //����������� ��� ������� ��������� � ������������������� ������
  if(data>start && data<end) nbr = PHONE1;
  else 
  {
   EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); 
   TEMP &= (1<<14);
   if(TEMP !=0)
   {
    READ_PHONE_EEPROM(PHONE_NUMBER, PHONE2); //���������� ������ �� EEPROM 
    data = strstr(RxBuffer , PHONE_NUMBER); //����������� ��� ������� ��������� � ������������������� ������
    if(data>start && data<end) nbr = PHONE2;
   }
  }
//------------------------------------------------------------------------------  
 if(nbr != 0)
 {
  SMS_Command_PARSER(nbr);
 }
} 
 SendString_InUnit("AT+CMGD=0,4\r\n"); 
delay_ms(100);

}
 
Reset_rxDMA_ClearBufer(); //����� ������  
}

//==============================================================================
static void READ_PHONE_EEPROM(char *ph, uint8_t num )//���������� ����������� ������ �� EEPROM
{
 uint8_t i; 
for(i=0;i<6;i++) //���������� ������ �� ��PRPOM
     {
       EE_ReadVariable(VirtAddVarTab[num+i], &TEMP);
      *ph++ = (uint8_t)TEMP;
      *ph++ = (uint8_t)(TEMP>>8);
     }
}
//==============================================================================
static void SMS_Command_PARSER(uint8_t num) //������ ��� ������
{
  char *s1, *s2;
  char code;
  uint8_t error_stat = 0;
  char str[6];
  
  s1 = strchr(RxBuffer , '#');
  if(s1 == NULL) SEND_SMS(COMMAND_ERROR, num);
  else
  {
 do{ 
   s1++;
   code = *s1++;
   switch(code)
   {
//------------------------------------------------------------------------------
   case '0':
     SEND_SMS(RESET_OK, num);
     CLEAR_EEPROM(); //������� ���� �������� � EEPROM
     error_stat = 99; //��� ������� ������
     break;
//------------------------------------------------------------------------------
   case '1':  //�����1
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~((uint16_t) 0x0F);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= ((uint16_t) 0x01);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= ((uint16_t) 0x02);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= ((uint16_t) 0x03);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case '2':  //�����2
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x0F)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= (((uint16_t) 0x01)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= (((uint16_t) 0x02)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= (((uint16_t) 0x03)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case '3':  //�����3
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x0F)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= (((uint16_t) 0x01)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= (((uint16_t) 0x02)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= (((uint16_t) 0x03)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case '4':  //�����4
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST1], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x0F)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= (((uint16_t) 0x01)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= (((uint16_t) 0x02)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= (((uint16_t) 0x03)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST1], TEMP);
       break;
       
       //.......................................................................
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case '5':  //�����5
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~((uint16_t) 0x0F);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= ((uint16_t) 0x01);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= ((uint16_t) 0x02);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= ((uint16_t) 0x03);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case '6':  //�����6
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x0F)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= (((uint16_t) 0x01)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= (((uint16_t) 0x02)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= (((uint16_t) 0x03)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
       case '4':     //���������� �����
       TEMP |= (((uint16_t) 0x04)<<4);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................     
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------ 
   case '7':  //�����7
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x0F)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= (((uint16_t) 0x01)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= (((uint16_t) 0x02)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= (((uint16_t) 0x03)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '4':     //����������
       TEMP |= (((uint16_t) 0x04)<<8);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------ 
   case '8':  //�����8
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SL_ST2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x0F)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������� 
       TEMP |= (((uint16_t) 0x01)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //����� 
       TEMP |= (((uint16_t) 0x02)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //��������� 
       TEMP |= (((uint16_t) 0x03)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................
     case '4':     //��������������
       TEMP |= (((uint16_t) 0x04)<<12);
       EE_WriteVariable(VirtAddVarTab[SL_ST2], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------ 
   case '9':  //~220V/40V
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~((uint16_t) 0x01);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������         
       TEMP |= ((uint16_t) 0x01);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------   
   case 'A':  //12V
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x01)<<1);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������         
       TEMP |= (((uint16_t) 0x01)<<1);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------ 
   case 'B':  //TEMP2
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x01)<<2);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������         
       TEMP |= (((uint16_t) 0x01)<<2);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------ 
   case 'C':  //OUT1
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS_2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~0x000F;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //������� �������         
       TEMP |= 0x0001;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //�������� �������         
       TEMP |= 0x0002;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //�����������         
       TEMP |= 0x0003;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '4':     //���������� �������         
       TEMP |= 0x0004;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case 'D':  //OUT2
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS_2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~0x00F0;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //������� �������         
       TEMP |= (((uint16_t) 0x01)<<4);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //�������� �������         
       TEMP |= (((uint16_t) 0x02)<<4);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //�����������         
       TEMP |= (((uint16_t) 0x03)<<4);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '4':     //���������� �������         
       TEMP |= (((uint16_t) 0x04)<<4);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case 'E':  //OUT_Relay1
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS_2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~0x0F00;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //������� �������         
       TEMP |= (((uint16_t) 0x01)<<8);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //�������� �������         
       TEMP |= (((uint16_t) 0x02)<<8);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //�����������         
       TEMP |= (((uint16_t) 0x03)<<8);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //....................................................................... 
      case '4':     //���������� �������         
       TEMP |= (((uint16_t) 0x04)<<8);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '5':     //���������        
       TEMP |= (((uint16_t) 0x05)<<8);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................      
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case 'F':  //OUT_Relay2
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS_2], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~0xF000;
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '1':     //������� �������         
       TEMP |= (((uint16_t) 0x01)<<12);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '2':     //�������� �������         
       TEMP |= (((uint16_t) 0x02)<<12);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '3':     //�����������         
       TEMP |= (((uint16_t) 0x03)<<12);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '4':     //���������� �������         
       TEMP |= (((uint16_t) 0x04)<<12);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................
     case '5':     //���������        
       TEMP |= (((uint16_t) 0x05)<<12);
       EE_WriteVariable(VirtAddVarTab[SETTINGS_2], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------ 
//------------------------------------------------------------------------------   
   case 'N':  //���������� ���������� �� �������� ����
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x01)<<11);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������         
       TEMP |= (((uint16_t) 0x01)<<11);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------   
   case 'S':  //�������� ����� � ��������
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x01)<<15);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������         
       TEMP |= (((uint16_t) 0x01)<<15);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------     
   case 'T':  //����� ������������ ��� ����������
     
     for(uint8_t i=0;i<5;i++)
     {
      if(i>3)
      {
       error_stat = 1; //������
       break;
      }
      else
      {
       if(isalnum(*s1) == 1) str[i] =*s1++;
       else if((*s1 == 0) || (*s1 == '#') || (*s1 == '.') || (*s1 == ','))
            {
            TEMP = atoi(str);  
            EE_WriteVariable(VirtAddVarTab[Q1], TEMP);
            break;
            }         
      }
     }
     if(((*s1 == '.') || (*s1 == ',')) && (error_stat != 1))
     {
      for(uint8_t i=0;i<6;i++) str[i] = 0;
      for(uint8_t i=0;i<5;i++)
     {
      if(i>3)
      {
       error_stat = 1; //������
       break;
      }
      else
      {
       if(isalnum(*s1) == 1) str[i] =*s1++;
       else if((*s1 == 0) || (*s1 == '#') )
            {
            TEMP = atoi(str);  
            EE_WriteVariable(VirtAddVarTab[Q2], TEMP);
            break;
            }         
      }
     }
     }
     for(uint8_t i=0;i<6;i++) str[i] = 0;
     
     break;
//------------------------------------------------------------------------------     
   case 'P':
     SEND_SMS(OPTIONS, num);
     error_stat = 88;
     break; 
//------------------------------------------------------------------------------ 
   case 'Z':  //�������� ����� ����������
     code = *s1;
     EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       TEMP &= ~(((uint16_t) 0x01)<<3);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................
     case '1':     //���������         
       TEMP |= (((uint16_t) 0x01)<<3);
       EE_WriteVariable(VirtAddVarTab[SETTINGS], TEMP);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------
   case 'G': //����������
     if(isalnum(*s1) == 1)
     {
       TEMP = (((*s1-0x30)*125)/10);
       EE_WriteVariable(VirtAddVarTab[GIST], TEMP);
     }
     else error_stat = 1; //������
     break; 
//------------------------------------------------------------------------------
   case 'H':
     if(isalnum(*s1) == 1) str[0] =*s1++;
     else error_stat = 1;
     
     if((isalnum(*s1) == 1) && (error_stat != 1)) str[1] =*s1++;
     else error_stat = 1;
     
     if(isalnum(*s1) == 1)  error_stat = 1;
     else
     {
      TEMP = atoi(str);  
      TEMP = ((TEMP *125)/10)+610;
      EE_WriteVariable(VirtAddVarTab[TERM], TEMP);
     }
     break; 
//------------------------------------------------------------------------------ 
   case 'X':  //���������� �������� �����
     code = *s1;     
     switch(code)
     {
       //.......................................................................
     case '0':     //����������         
       DiviceStatus |= (1<<6);
       break;
       
       //.......................................................................
     case '1':     //���������         
       DiviceStatus &= ~(1<<6);
       break;
       
       //.......................................................................       
     default:
     error_stat = 1; //������
     break;       
     }
     break;
//------------------------------------------------------------------------------ 
   case 'M':
     BALLANSE(num); //������� �������� ��������
     error_stat = 88;
     break; 
//------------------------------------------------------------------------------     
     
   default:
     error_stat = 1; //������
     break;
   } //end switch
   
 s2 =  strchr(s1 , '#');
 s1=s2;
   
  }while((s1 != NULL) && (error_stat == 0));
    
   if(error_stat == 0) SEND_SMS(COMMAND_COMPLETE, num);
   else if(error_stat == 1) SEND_SMS(COMMAND_ERROR, num);
  } //end else 
}
//==============================================================================
void SendData_onServer(SMS_TypeDef sms)  //������� �������� ������ �� ������

{
 
  FunctionalState execute = DISABLE; //������ ������ ������� � ��������
  uint8_t cntr; //�������
 // char out[]="ffff"; 


  
  Reset_rxDMA_ClearBufer(); //����� ������
   
for(uint8_t cnt=0;cnt<3;cnt++) //������� ������� ��������
  {  
    SendString_InUnit("AT+WIPCREATE=2,1,"); //�������� ������ 
    SendString_InUnit(SERVER);
    SendString_InUnit(PORT);
    
     for (cntr=0;cntr<250;cntr++)
    {
      if(strstr(RxBuffer , "+WIPREADY: 2,1") !=NULL) //���� ����� �������� 
      {
       SendString_InUnit("AT+WIPDATA=2,1,1\r\n"); //�������� � ����� �������� ������
       delay_ms(100);
       if(strstr(RxBuffer , "CONNECT") !=NULL) //���� ����� ����������� 
       {  
         //STATUS.GSM_DataMode = ENABLE;
        Reset_rxDMA_ClearBufer(); //����� ������
        
/***************************�������� ���������� �� ������*************************************/
        
        SendString_InUnit("POST /bb/gasStatus HTTP/1.1\r\n" ); //��������� POST �������
        SendString_InUnit("Host: gs.ggsbarnaul.ru\r\n");
        SendString_InUnit("Content-Type: application/x-www-form-urlencoded\r\n" );
        SendString_InUnit("Content-Length: 23\r\n" );
        
        SendString_InUnit("\r\n"); //����� ������
                    
        SendString_InUnit("s_n="); //�������� ��������� ������
        SendString_InUnit(SERIAL_NUMBER );
        
        SendString_InUnit("&alarm=");
        switch(sms)
        {
        case GAS_ALARM:
          SendString_InUnit("10");
          break;
        case SMOK:
          SendString_InUnit("11");
          break;  
        case FIRE:
          SendString_InUnit("12");
          break;
        case TEMP_ALARM:
          SendString_InUnit("13");
          break;    
         case VOLTAGE_MISSING_3H:
          SendString_InUnit("14");
          break;  
        }

        SendString_InUnit("\r\n"); //����� ������
        SendString_InUnit("\r\n"); //����� ������
/*************************�������� ������ �� �������***************************/       
         for (cntr=0;cntr<13;cntr++)
         {
           
           if(strstr(RxBuffer , "*00#") !=NULL)
          {
           
           execute = ENABLE; 
           break;
          }
          delay_ms(1000);
          //IWDG_ReloadCounter(); //����� �������� ����������� �������
         }
 /***********************�������� ������****************************************/
        delay_ms(100);   
        Reset_rxDMA_ClearBufer(); //����� ������
        
        SendString_InUnit("+++");
        
        for(uint8_t i=0;i<10; i++)
        {
         delay_ms(100);
         if(strstr(RxBuffer , "OK") !=NULL)
         {
           SendString_InUnit("AT+WIPCLOSE=2,1\r\n");
           delay_ms(100);
           //STATUS.GSM_DataMode = DISABLE;
           break; 
         }
        }
        
       }
       break;
      }
      if(strstr(RxBuffer , "ERROR") !=NULL)
      {
 
       break;
      }  
      delay_ms(100);
     // IWDG_ReloadCounter(); //����� �������� ����������� �������
    }
    
 Reset_rxDMA_ClearBufer(); //����� ������ 

if(execute == ENABLE)
{
 SendDataError = 0; 
 break; //���� ������ �������� ����� �� ����� ������� ��������
}
}
 

 if(execute != ENABLE) SendDataError++;
}

//===============================================================================
void BALLANSE(uint8_t nbr) //������� �������� ��������
{
 char *s1, *s2; 
 uint8_t S, k;
 char bal_buf[128];
 FunctionalState Send = DISABLE; //������ �������� ���������
 char PHONE_NUMBER[] = "123456789012"; //������ ������
 char PDU_PHONE_NUMBER[] = "123456789012"; //������ ������ � PDU �������
 char LEN[5];
 
 
 delay_ms(100);
 Reset_rxDMA_ClearBufer(); //����� ������
SendString_InUnit("AT+CUSD=1,\"*100#\"\r\n"); //������ ��������

 for(uint8_t i=0; i<100;i++) //���� ������ �� ������
 {
  delay_ms(100);
  s2 = strstr(RxBuffer, "+CUSD:");
  if(s2 != NULL) //���� ���� ��������
  {
  s1 = strchr(s2 , '"'); //����� ������ ����� ������
  s2 = strstr(RxBuffer, "0440"); //����� ����� '�'
  if((s1!=NULL) && (s2!=NULL)) 
  {
    s1++;
    s2 = s2+4;
    S = ((s2-s1)/2)+13; //��������� ������ ���������
    for(k=0;k<128;k++) bal_buf[k] =0;
    k=0;
    while(s1 != s2) bal_buf[k++] = *s1++; //�������� ������ � ������
    
    READ_PHONE_EEPROM(PHONE_NUMBER, nbr); //���������� ������ �� EEPROM
    for(uint8_t j=0; j<10; j+=2) PDU_PHONE_NUMBER[j] = PHONE_NUMBER[j+2]; //���������� ������� PDU
    PDU_PHONE_NUMBER[10] = 'F';
    for(uint8_t j=1; j<12; j+=2) PDU_PHONE_NUMBER[j] = PHONE_NUMBER[j];
    
    Reset_rxDMA_ClearBufer(); //����� ������
    delay_ms(500);
    SendString_InUnit("AT+CMGF=0\r\n"); //��������� PDU ������
    delay_ms(100); 
    
    for(uint8_t j=0;j<5;j++) //������� �������� ���������
    {
 
      SendString_InUnit("AT+CMGS="); //������� �������� ���������
      sprintf(LEN , "%02d\r\n", S);
      SendString_InUnit(LEN); //����� ������ ���������
      delay_ms(100);
 
      if(strchr(RxBuffer , '>')!=NULL)
      {
       Reset_rxDMA_ClearBufer(); //����� ������
 
       SendString_InUnit("0001000B91"); //��������� PDU
       SendString_InUnit(PDU_PHONE_NUMBER); //����� � ������� PDU
       SendString_InUnit("0008"); //���������
       
       for(k=0;k<5;k++) LEN[k] =0;//������� ������ ������
       S=S-13;
       sprintf(LEN , "%02x", S);
       SendString_InUnit(LEN); //����� ������ ����� ������
       
       SendString_InUnit(bal_buf); //���� ����� ������
       
        delay_ms(50);
       SendString_InUnit("\x1A"); //������� �������� ���������
       
       for( k=0; k<100; k++)
       { 
        delay_ms(50);
        if(strstr(RxBuffer , "OK") != NULL) 
        {
         Send = ENABLE;
         break;
        }
        delay_ms(50);
        if(strstr(RxBuffer , "ERROR") != NULL) break;
       } 
       
      } //end if 
      Reset_rxDMA_ClearBufer(); //����� ������
      if(Send == ENABLE) break;
      delay_ms(2000);
    }//end for ����� �������
  SendString_InUnit("AT+CMGF=1\r\n"); //������������ � ����� ��������� ���������
  delay_ms(100);
  Reset_rxDMA_ClearBufer(); //����� ������ 
    
  }//end if
    break;
  }//����� if  
 } //����� for
}

/*******************************************************************************
********************************************************************************
**                                                                            **
**                        ����� ���������                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/