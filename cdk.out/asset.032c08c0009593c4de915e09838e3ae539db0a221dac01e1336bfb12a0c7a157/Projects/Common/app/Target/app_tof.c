/**
  ******************************************************************************
  * @file          : app_tof.c
  * @author        : IMG SW Application Team
  * @brief         : This file provides code for the configuration
  *                  of the STMicroelectronics.X-CUBE-TOF1.3.3.0 instances.
  ******************************************************************************
  *
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_tof.h"
#include "app_utils.h"
#include "tof_macros.h"
#include <stdio.h>


#include "b_u585i_iot02a.h"
#include "custom_ranging_sensor.h"
//#define interrupt_mode

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* uncomment following to use directly the bare driver instead of the BSP */
/* #define USE_BARE_DRIVER */
#define TIMING_BUDGET (30U) /* 5 ms < TimingBudget < 100 ms */
#define RANGING_FREQUENCY (5U) /* Ranging frequency Hz (shall be consistent with TimingBudget value) */
#define POLLING_PERIOD (1000U/RANGING_FREQUENCY) /* refresh rate for polling mode (milliseconds) */

/* Private variables ---------------------------------------------------------*/

RANGING_SENSOR_ProfileConfig_t Profile;
RANGING_SENSOR_Result_t Result;
#ifndef USE_BARE_DRIVER
static RANGING_SENSOR_Capabilities_t Cap;
#endif

extern AppConfig_TypeDef App_Config;
static int32_t status = 0;
volatile uint8_t ToF_EventDetected = 0;
int f = 0;
/* Private function prototypes -----------------------------------------------*/
#ifdef USE_BARE_DRIVER
static uint8_t map_target_status(uint8_t status);
static int32_t convert_data_format(VL53L5CX_Object_t *pObj,
    VL53L5CX_ResultsData *data, RANGING_SENSOR_Result_t *pResult);
#endif
static void MX_VL53L5CX_SimpleRanging_Init(void);
int32_t MX_VL53L5CX_SimpleRanging_Process();

void toggle_resolution(void);
void toggle_signal_and_ambient(void);
void clear_screen(void);
void display_commands_banner(void);
void handle_cmd(uint8_t cmd);
uint8_t get_key(void);
uint32_t com_has_data(void);

void MX_TOF_Init(void)
{
  /* USER CODE BEGIN SV */

  /* USER CODE END SV */

  /* USER CODE BEGIN TOF_Init_PreTreatment */

  /* USER CODE END TOF_Init_PreTreatment */

  /* Initialize the peripherals and the TOF components */

  MX_VL53L5CX_SimpleRanging_Init();

  /* USER CODE BEGIN TOF_Init_PostTreatment */

  /* USER CODE END TOF_Init_PostTreatment */
}

/*
 * LM background task
 */
void MX_TOF_Process(void)
{
  /* USER CODE BEGIN TOF_Process_PreTreatment */

  /* USER CODE END TOF_Process_PreTreatment */

  MX_VL53L5CX_SimpleRanging_Process(&App_Config);

  /* USER CODE BEGIN TOF_Process_PostTreatment */


  /* USER CODE END TOF_Process_PostTreatment */
}

static void MX_VL53L5CX_SimpleRanging_Init(void)
{
  /* Initialize Virtual COM Port */
  //BSP_COM_Init(COM1);

  status = CUSTOM_RANGING_SENSOR_Init(CUSTOM_VL53L5CX);

  if (status != BSP_ERROR_NONE)
  {
    printf("CUSTOM_RANGING_SENSOR_Init failed\n");
    while(1);
  }
}

#ifdef USE_BARE_DRIVER
static void MX_VL53L5CX_SimpleRanging_Process(void)
{
  VL53L5CX_Object_t *pL5obj = CUSTOM_RANGING_CompObj[CUSTOM_VL53L5CX];
  static VL53L5CX_ResultsData data;
  uint8_t NewDataReady = 0;

  Profile.RangingProfile = RS_PROFILE_4x4_CONTINUOUS;
  Profile.TimingBudget = TIMING_BUDGET; /* 5 ms < TimingBudget < 100 ms */
  Profile.Frequency = RANGING_FREQUENCY; /* Ranging frequency Hz (shall be consistent with TimingBudget value) */
  Profile.EnableAmbient = 0; /* Enable: 1, Disable: 0 */
  Profile.EnableSignal = 0; /* Enable: 1, Disable: 0 */

  pL5obj->IsAmbientEnabled = Profile.EnableAmbient;
  pL5obj->IsSignalEnabled = Profile.EnableSignal;

  /*
     use case VL53L5CX_PROFILE_4x4_CONTINUOUS:
  */
  status = vl53l5cx_set_resolution(&(pL5obj->Dev), VL53L5CX_RESOLUTION_4X4);
  status |= vl53l5cx_set_ranging_mode(&(pL5obj->Dev), VL53L5CX_RANGING_MODE_CONTINUOUS);
  status |= vl53l5cx_set_integration_time_ms(&(pL5obj->Dev), TIMING_BUDGET);
  status |= vl53l5cx_set_ranging_frequency_hz(&(pL5obj->Dev), RANGING_FREQUENCY);

  if (status != VL53L5CX_STATUS_OK)
  {
    printf("ERROR : Configuration programming error!\n\n");
    while (1);
  }

  status = vl53l5cx_start_ranging(&(pL5obj->Dev));
  if (status != VL53L5CX_STATUS_OK)
  {
    printf("vl53l5cx_start_ranging failed\n");
    while(1);
  }

  while (1)
  {
    /* polling mode */
    (void)vl53l5cx_check_data_ready(&(pL5obj->Dev), &NewDataReady);

    if (NewDataReady != 0)
    {
      status = vl53l5cx_get_ranging_data(&(pL5obj->Dev), &data);

      if (status == VL53L5CX_STATUS_OK)
      {
        /*
         Convert the data format to Result format.
         Note that you can print directly from data format
        */
        if (convert_data_format(pL5obj, &data, &Result) < 0)
        {
          printf("convert_data_format failed\n");
          while(1);
        }
        print_result(&Result);
      }
    }

    if (com_has_data())
    {
      handle_cmd(get_key());
    }

    HAL_Delay(POLLING_PERIOD);
  }
}
#else
int32_t  MX_VL53L5CX_SimpleRanging_Process()
{

	if (f == 0)
	{
	 f = 1;
	 uint32_t Id;

  	  CUSTOM_RANGING_SENSOR_ReadID(CUSTOM_VL53L5CX, &Id);
  	  CUSTOM_RANGING_SENSOR_GetCapabilities(CUSTOM_VL53L5CX, &Cap);

  	  Profile.RangingProfile = RS_PROFILE_8x8_CONTINUOUS;
  	  Profile.TimingBudget = TIMING_BUDGET; /* 5 ms < TimingBudget < 100 ms */
  	  Profile.Frequency = RANGING_FREQUENCY; /* Ranging frequency Hz (shall be consistent with TimingBudget value) */
  	  Profile.EnableAmbient = 0; /* Enable: 1, Disable: 0 */
  	  Profile.EnableSignal = 1; /* Enable: 1, Disable: 0 */

  	  /* set the profile if different from default one */
  	  CUSTOM_RANGING_SENSOR_ConfigProfile(CUSTOM_VL53L5CX, &Profile);

  	  status = CUSTOM_RANGING_SENSOR_Start(CUSTOM_VL53L5CX, RS_MODE_BLOCKING_CONTINUOUS);
	}

	#ifndef interrupt_mode
		/* polling mode */
	status = CUSTOM_RANGING_SENSOR_GetDistance(CUSTOM_VL53L5CX);



	#else

	  if(ToF_EventDetected)
	  {

		  ToF_EventDetected = 0;


		  status = CUSTOM_RANGING_SENSOR_GetDistance(CUSTOM_VL53L5CX);

		  if (status == BSP_ERROR_NONE)
		  {
			//print_result(&Result);

			//status = pre_processing(&Result);

		  }
/*
		  if (com_has_data())
		  {
			handle_cmd(get_key());
		  }*/
	  }


return status;

#endif

}
#endif /* USE_BARE_DRIVER */

void toggle_resolution(void)
{
  CUSTOM_RANGING_SENSOR_Stop(CUSTOM_VL53L5CX);

  switch (Profile.RangingProfile)
  {
    case RS_PROFILE_4x4_AUTONOMOUS:
      Profile.RangingProfile = RS_PROFILE_8x8_AUTONOMOUS;
      break;

    case RS_PROFILE_4x4_CONTINUOUS:
      Profile.RangingProfile = RS_PROFILE_8x8_CONTINUOUS;
      break;

    case RS_PROFILE_8x8_AUTONOMOUS:
      Profile.RangingProfile = RS_PROFILE_4x4_AUTONOMOUS;
      break;

    case RS_PROFILE_8x8_CONTINUOUS:
      Profile.RangingProfile = RS_PROFILE_4x4_CONTINUOUS;
      break;

    default:
      break;
  }

  CUSTOM_RANGING_SENSOR_ConfigProfile(CUSTOM_VL53L5CX, &Profile);
  CUSTOM_RANGING_SENSOR_Start(CUSTOM_VL53L5CX, RS_MODE_BLOCKING_CONTINUOUS);
}

void toggle_signal_and_ambient(void)
{
  CUSTOM_RANGING_SENSOR_Stop(CUSTOM_VL53L5CX);

  Profile.EnableAmbient = (Profile.EnableAmbient) ? 0U : 1U;
  Profile.EnableSignal = (Profile.EnableSignal) ? 0U : 1U;

  CUSTOM_RANGING_SENSOR_ConfigProfile(CUSTOM_VL53L5CX, &Profile);
  CUSTOM_RANGING_SENSOR_Start(CUSTOM_VL53L5CX, RS_MODE_BLOCKING_CONTINUOUS);
}

void clear_screen(void)
{
  /* clear Screen : 27 is ESC command */
  printf("%c[2J", 27);
}

void display_commands_banner(void)
{
  /* clear screen */

  printf("%c[2H", 27);

  printf("VL53L5CX Simple Ranging demo application\r\n");
#ifdef USE_BARE_DRIVER
  printf("Using direct calls to VL53L5CX bare driver API\r\n");
#endif
  printf("Polling mode\n");
  printf("----------------------------------------\r\n\r\n");

  printf("Use the following keys to control application\r\n");
  printf(" 'r' : change resolution\r\n");
  printf(" 's' : enable signal and ambient\r\n");
  printf(" 'c' : clear screen\r\n");
  printf("\r\n");
}

void handle_cmd(uint8_t cmd)
{
  switch (cmd)
  {
    case 'r':
      toggle_resolution();
      clear_screen();
      break;

    case 's':
      toggle_signal_and_ambient();
      clear_screen();
      break;

    case 'c':
      clear_screen();
      break;

    default:
      break;
  }
}

/*uint8_t get_key(void)
{
  uint8_t cmd = 0;

  HAL_UART_Receive(&hcom_uart[0U], &cmd, 1, HAL_MAX_DELAY);

  return cmd;
}*/

/*uint32_t com_has_data(void)
{
  return __HAL_UART_GET_FLAG(&hcom_uart[0U], UART_FLAG_RXNE);;
}*/

#ifdef USE_BARE_DRIVER
static uint8_t map_target_status(uint8_t status)
{
  uint8_t ret;

  if ((status == 5U) || (status == 9U))
  {
    ret = 0U; /* ranging is OK */
  }
  else if (status == 0U)
  {
    ret = 255U; /* no update */
  }
  else
  {
    ret = status; /* return device status otherwise */
  }

  return ret;
}

static int32_t convert_data_format(VL53L5CX_Object_t *pObj,
    VL53L5CX_ResultsData *data, RANGING_SENSOR_Result_t *pResult)
{
  int32_t ret;
  uint8_t i, j;
  uint8_t resolution;
  uint8_t target_status;

  if ((pObj == NULL) || (pResult == NULL))
  {
    ret = VL53L5CX_INVALID_PARAM;
  }
  else if (vl53l5cx_get_resolution(&pObj->Dev, &resolution) != VL53L5CX_STATUS_OK)
  {
    ret = VL53L5CX_ERROR;
  }
  else
  {
    pResult->NumberOfZones = resolution;

    for (i = 0; i < resolution; i++)
    {
      pResult->ZoneResult[i].NumberOfTargets = data->nb_target_detected[i];

      for (j = 0; j < data->nb_target_detected[i]; j++)
      {
        pResult->ZoneResult[i].Distance[j] = (uint32_t)data->distance_mm[(VL53L5CX_NB_TARGET_PER_ZONE * i) + j];

        /* return Ambient value if ambient rate output is enabled */
        if (pObj->IsAmbientEnabled == 1U)
        {
          /* apply ambient value to all targets in a given zone */
          pResult->ZoneResult[i].Ambient[j] = (float_t)data->ambient_per_spad[i];
        }
        else
        {
          pResult->ZoneResult[i].Ambient[j] = 0.0f;
        }

        /* return Signal value if signal rate output is enabled */
        if (pObj->IsSignalEnabled == 1U)
        {
          pResult->ZoneResult[i].Signal[j] =
            (float_t)data->signal_per_spad[(VL53L5CX_NB_TARGET_PER_ZONE * i) + j];
        }
        else
        {
          pResult->ZoneResult[i].Signal[j] = 0.0f;
        }

        target_status = data->target_status[(VL53L5CX_NB_TARGET_PER_ZONE * i) + j];
        pResult->ZoneResult[i].Status[j] = map_target_status(target_status);
      }
    }

    ret = VL53L5CX_OK;
  }

  return ret;
}
#endif

#ifdef __cplusplus
}
#endif
