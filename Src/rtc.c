/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

	/**Initialize RTC Only 
    */
	hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
	hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *rtcHandle)
{

	if (rtcHandle->Instance == RTC)
	{
		/* USER CODE BEGIN RTC_MspInit 0 */

		/* USER CODE END RTC_MspInit 0 */
		HAL_PWR_EnableBkUpAccess();
		/* Enable BKP CLK enable for backup registers */
		__HAL_RCC_BKP_CLK_ENABLE();
		/* RTC clock enable */
		__HAL_RCC_RTC_ENABLE();
		/* USER CODE BEGIN RTC_MspInit 1 */

		/* USER CODE END RTC_MspInit 1 */
	}
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef *rtcHandle)
{

	if (rtcHandle->Instance == RTC)
	{
		/* USER CODE BEGIN RTC_MspDeInit 0 */

		/* USER CODE END RTC_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_RTC_DISABLE();
		/* USER CODE BEGIN RTC_MspDeInit 1 */

		/* USER CODE END RTC_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */
/*============================================================================
 由RTC时间转换成Unix时间戳
 return：Unix时间戳
 ============================================================================*/
uint32_t getUnixTick()
{
	uint32_t retval;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	struct tm time_t; 
	memset((void *)&time_t,0,sizeof(time_t));

	HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
	

	time_t.tm_sec = time.Seconds;
	time_t.tm_min = time.Minutes;
	time_t.tm_hour= time.Hours;
	time_t.tm_year= date.Year + 100;
	time_t.tm_mon = date.Month - 1;
	time_t.tm_mday= date.Date;

	retval = my_mktime(&time_t);

	DBG_PRT("unix tick:%u\n",retval);

	return retval;
}

/*============================================================================
 由gprs的同步网络时间的返回字符串，获得Unix时间戳
 之后设置RTC时间
 timeStr：gprs时间字符串
 ============================================================================*/
void gprs_getTime(uint8_t *timeStr)
{
	uint8_t label_cnt = 0;				//时间字符串标点符号个数
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
	uint8_t *p,*p1;

	p = (uint8_t *)strstr((char const*)timeStr,"Update To ");
	if(p != 0)
	{
		p += strlen("Update To ");
		DBG_PRT("time str:%s\n",p);
		p1 = p;
		while(*p1 != '\r' && *p1 != 0x00 && (p1 - p) < 256)
		{
			if(*p1 < '0' || *p1 > '9')
			{
				*p1 = 0;
				label_cnt++;
			}
			p1++;
		}

		//标准返回里，共有五个标点符号，否则字符串不对，不予更新时钟
		if(label_cnt != 5)
		{
			DBG_PRT("time str convert Err!\n");
			return;
		}

		date.Year = atoi((const char *)p) - 2000;
		p += 5;
		date.Month = atoi((const char *)p);
		p += 3;
		date.Date = atoi((const char *)p);
		p += 3;
		time.Hours = atoi((const char *)p);
		p += 3;
		time.Minutes =atoi((const char *)p);
		p += 3;
		time.Seconds = atoi((const char *)p);

		//判断提取有效性
		if(date.Year > 99
			|| date.Month > 12
			|| date.Date < 1 || date.Date > 31
			|| time.Hours > 23
			|| time.Minutes > 59
			|| time.Seconds > 59)
		{
			DBG_PRT("time str convert Err!\n");
			return;
		}

		HAL_RTC_SetDate(&hrtc,&date,RTC_FORMAT_BIN);
		HAL_RTC_SetTime(&hrtc,&time,RTC_FORMAT_BIN);
	}

	DBG_PRT("time str convert,RTC set OK!\n");
	
}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
