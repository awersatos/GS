/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "main.h"
#include "eeprom.h"
#include "GSM.h"    
#include <string.h>
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  if(delay_Counter != 0) delay_Counter--;
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

void EXTI9_5_IRQHandler(void) //Внешние прерывания линии 5-9
{
 if(EXTI_GetITStatus(EXTI_Line5) != RESET) //220V Вход газоанализатора
  {
  EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     if((TEMP & 1) !=0)
     {
     EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);  
     TEMP|=(1<<8);
     EE_WriteVariable(VirtAddVarTab[ERROR], TEMP);
     }
    
  EXTI_ClearITPendingBit(EXTI_Line5); //Очистка флага прерывания
  }
  
//------------------------------------------------------------------------------  
  
 if(EXTI_GetITStatus(EXTI_Line6) != RESET) //12V Вход газоанализатора
  {
  EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP);
     if((TEMP & (1<<1)) !=0)
     {
     EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);  
     TEMP|=(1<<8);
     EE_WriteVariable(VirtAddVarTab[ERROR], TEMP);
     }  
  EXTI_ClearITPendingBit(EXTI_Line6); //Очистка флага прерывания
  }
//------------------------------------------------------------------------------
if(EXTI_GetITStatus(EXTI_Line9) != RESET) //Установка телефонных номеров
  {
    
    delay_ms(500);
    SendString_InUnit("ATH\r\n"); 
   if(DiviceStatus == 1 )
   {
    if(strstr(RxBuffer , "RING") !=NULL) 
    {
      char *s;
      char b0, b1;
      s = strstr(RxBuffer , "+79");
      EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
      TEMP &= (1<<15);
      if(TEMP == 0)
      {
       
        for(uint8_t i=0;i<6;i++)
       {
        b0 = *s++;
        b1 = *s++;
        TEMP = ((uint16_t)b1<<8) | (uint16_t)b0;
        EE_WriteVariable(VirtAddVarTab[PHONE1+i], TEMP);
       
       }
      TEMP = (1<<15);
      EE_WriteVariable(VirtAddVarTab[ERROR], TEMP);
      SET_ALARM(120);
      delay_ms(500);
      SEND_SMS(NUMBER_SET, PHONE1);
      }
      else
      {
       delay_ms(10); 
       EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP); 
       TEMP &= (1<<14);
        if(TEMP == 0)
       {
       
         for(uint8_t i=0;i<6;i++)
        {
         b0 = *s++;
         b1 = *s++;
         TEMP = ((uint16_t)b1<<8) | (uint16_t)b0;
         EE_WriteVariable(VirtAddVarTab[PHONE2+i], TEMP);
       
        }
        EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
        TEMP |= (1<<14);
        EE_WriteVariable(VirtAddVarTab[ERROR], TEMP);
        delay_ms(500);
        SEND_SMS(NUMBER_SET, PHONE2);
       }
      
      }
    //  EE_ReadVariable(VirtAddVarTab[ERROR], &TEMP);
      
    }
     
   }
    delay_ms(500);
    Reset_rxDMA_ClearBufer(); //Сброс буфера
    EXTI_ClearITPendingBit(EXTI_Line9); //Очистка флага прерывания
  }
}
//==============================================================================
void TIM4_IRQHandler(void) //Прерывания таймера 3
{
//if (timmin[index]!=0)
//timmin[index] = timmin[index]+1;
if(Led1_state > 3)
{
  switch(Led1_state)
  {
    case BLINK_RED:
     if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_2) == 0)
     {
      GPIO_SetBits(GPIOC, GPIO_Pin_2);
      GPIO_ResetBits(GPIOC, GPIO_Pin_3); 
     }
    else
     {
     GPIO_ResetBits(GPIOC, GPIO_Pin_3);
     GPIO_ResetBits(GPIOC, GPIO_Pin_2); 
     } 
    break; 
    
    case BLINK_GREN:
     if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_3) == 0)
     {
      GPIO_SetBits(GPIOC, GPIO_Pin_3);
      GPIO_ResetBits(GPIOC, GPIO_Pin_2); 
     }
    else
     {
     GPIO_ResetBits(GPIOC, GPIO_Pin_3);
     GPIO_ResetBits(GPIOC, GPIO_Pin_2); 
     } 
    break;
    
    case BLINK_RED_GREEN:
     if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_3) == 0)
     {
      GPIO_SetBits(GPIOC, GPIO_Pin_3);
      GPIO_ResetBits(GPIOC, GPIO_Pin_2); 
     }
    else
     {
     GPIO_SetBits(GPIOC, GPIO_Pin_2);
     GPIO_ResetBits(GPIOC, GPIO_Pin_3); 
     } 
    break;       
    
  }
}

if(Led2_state > 3)
{
  switch(Led2_state)
  {
    case BLINK_RED:
     if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_4) == 0)
     {
      GPIO_SetBits(GPIOC, GPIO_Pin_4);
      GPIO_ResetBits(GPIOC, GPIO_Pin_5); 
     }
    else
     {
     GPIO_ResetBits(GPIOC, GPIO_Pin_5);
     GPIO_ResetBits(GPIOC, GPIO_Pin_4); 
     } 
    break; 
    
    case BLINK_GREN:
     if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_5) == 0)
     {
      GPIO_SetBits(GPIOC, GPIO_Pin_5);
      GPIO_ResetBits(GPIOC, GPIO_Pin_4); 
     }
    else
     {
     GPIO_ResetBits(GPIOC, GPIO_Pin_5);
     GPIO_ResetBits(GPIOC, GPIO_Pin_4); 
     } 
    break;
    
    case BLINK_RED_GREEN:
     if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_5) == 0)
     {
      GPIO_SetBits(GPIOC, GPIO_Pin_5);
      GPIO_ResetBits(GPIOC, GPIO_Pin_4); 
     }
    else
     {
     GPIO_SetBits(GPIOC, GPIO_Pin_4);
     GPIO_ResetBits(GPIOC, GPIO_Pin_5); 
     } 
    break;       
    
  }  
  
}



TIM_ClearITPendingBit(TIM4 , TIM_IT_Update); //Очистка флага прерывания
}
//==============================================================================
void DMA1_Channel1_IRQHandler(void)
{
 DMA_ClearITPendingBit(DMA1_IT_GL1); 
}
//==============================================================================
void RTC_IRQHandler(void) //Прерывания от часов реального времени
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    thermostat_timer++;
    RTC_ClearITPendingBit(RTC_FLAG_SEC);
  }
//------------------------------------------------------------------------------
  if (RTC_GetITStatus(RTC_IT_ALR) != RESET) //Прерывание ALARM
  {
   
   if((DiviceStatus & 0x01) == 1) DiviceStatus &= ~0x01;
   
    if(((DiviceStatus & (1<<2)) != 0) && ((DiviceStatus & (1<<1)) == 0))
   {SEND_SMS(VOLTAGE_MISSING_3H, PHONE1);
   }
   
   if((DiviceStatus & (1<<1)) != 0)
   {
    EE_ReadVariable(VirtAddVarTab[SETTINGS], &TEMP); 
    if((TEMP & (1<<11)) != 0)
    {
      SEND_SMS(VOLTAGE_MISSING, PHONE1);
     
    }
     DiviceStatus |= (1<<2);
     DiviceStatus &= ~(1<<1);
     SET_ALARM(10800); //Установка на 3 часа
   }
   
  
    RTC_ClearITPendingBit(RTC_IT_ALR);
  }

}
//==============================================================================
void EXTI1_IRQHandler(void) //Прерывание контроля питания
{
  delay_ms(10);
  if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 1) //Питание присутствует
  {
    LED(1, GREEN);
    if((DiviceStatus & (1<<2)) != 0)
    {
     SEND_SMS(VOLTAGE_ON, PHONE1); 
     
    }
    DiviceStatus &= ~(0x03<<1);
  }
  else //Питание отсутствует
  {
   LED(1, BLINK_RED); 
   DiviceStatus |= (1<<1);
   SET_ALARM(120);
  }
  
 EXTI_ClearITPendingBit(EXTI_Line1); //Очистка флага прерывания  
}
//==============================================================================
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
