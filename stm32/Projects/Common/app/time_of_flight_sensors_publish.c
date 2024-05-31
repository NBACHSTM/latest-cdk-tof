/*
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Derived from simple_sub_pub_demo.c
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <stdio.h>
#include "logging_levels.h"
#include "b_u585i_iot02a_errno.h"
/* define LOG_LEVEL here if you want to modify the logging level from the default */

#define LOG_LEVEL    LOG_INFO

#include "logging.h"


/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "kvstore.h"

#include "sys_evt.h"

#include "app_tof.h"
#include "custom_ranging_sensor.h"

/* MQTT library includes. */
#include "core_mqtt.h"

/* MQTT agent include. */
#include "core_mqtt_agent.h"
#include "mqtt_agent_task.h"

/* Subscription manager header include. */
#include "subscription_manager.h"
#include "app_network.h"
#include "ai_model_config.h"
/* Exported macro ------------------------------------------------------------*/
#define HANDPOSTURE_EXAMPLE_VERSION "0.0.1"

/* Exported constants --------------------------------------------------------*/
/* Table of classes for the NN model */

/* Includes ------------------------------------------------------------------*/
const char* classe_table[NB_CLASSES] = {"None" ,   "FlatHand" ,   "Like" ,   "Love" ,   "Dislike" ,   "BreakTime" , "CrossHands" ,   "Fist"};
/* Private variables ---------------------------------------------------------*/

extern RANGING_SENSOR_Result_t Result;
AppConfig_TypeDef App_Config;
extern int32_t CUSTOM_RANGING_SENSOR_Init(uint32_t Instance);
extern int32_t MX_VL53L5CX_SimpleRanging_Process(void);
extern int32_t * r;
void print_result(AppConfig_TypeDef *App_Config);
static int32_t status = 0;


#if (USE_CUSTOM_RANGING_VL53L5CX == 1U)
#define CUSTOM_VL53L5CX (0)
#endif

/**
 * @brief Size of statically allocated buffers for holding topic names and
 * payloads.
 */
#define MQTT_PUBLISH_MAX_LEN                 ( 200 )
#define MQTT_PUBLISH_PERIOD_MS               ( 100 )
#define MQTT_PUBLICH_TOPIC_STR_LEN           ( 256 )
#define MQTT_PUBLISH_BLOCK_TIME_MS           ( 300 )
#define MQTT_PUBLISH_NOTIFICATION_WAIT_MS    ( 1000 )
#define MQTT_NOTIFY_IDX                      ( 1 )
#define MQTT_PUBLISH_QOS                     ( MQTTQoS0 )


/*-----------------------------------------------------------*/

/**
 * @brief Defines the structure to use as the command callback context in this
 * demo.
 */
struct MQTTAgentCommandContext
{
    MQTTStatus_t xReturnStatus;
    TaskHandle_t xTaskToNotify;
    uint32_t ulNotificationValue;
};

/*-----------------------------------------------------------*/
static void prvPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                       MQTTAgentReturnInfo_t * pxReturnInfo )
{
    TaskHandle_t xTaskHandle = ( TaskHandle_t ) pxCommandContext;

    configASSERT( pxReturnInfo != NULL );

    uint32_t ulNotifyValue = pxReturnInfo->returnCode;

    if( xTaskHandle != NULL )
    {
        /* Send the context's ulNotificationValue as the notification value so
         * the receiving task can check the value it set in the context matches
         * the value it receives in the notification. */
        ( void ) xTaskNotifyIndexed( xTaskHandle,
                                     MQTT_NOTIFY_IDX,
                                     ulNotifyValue,
                                     eSetValueWithOverwrite );
    }
}

/*-----------------------------------------------------------*/

static BaseType_t prvPublishAndWaitForAck( MQTTAgentHandle_t xAgentHandle,
                                           const char * pcTopic,
                                           const void * pvPublishData,
                                           size_t xPublishDataLen )
{
    MQTTStatus_t xStatus;
    size_t uxTopicLen = 0;

    configASSERT( pcTopic != NULL );
    configASSERT( pvPublishData != NULL );
    configASSERT( xPublishDataLen > 0 );

    uxTopicLen = strnlen( pcTopic, UINT16_MAX );

    MQTTPublishInfo_t xPublishInfo =
    {
        .qos             = MQTT_PUBLISH_QOS,
        .retain          = 0,
        .dup             = 0,
        .pTopicName      = pcTopic,
        .topicNameLength = ( uint16_t ) uxTopicLen,
        .pPayload        = pvPublishData,
        .payloadLength   = xPublishDataLen
    };

    MQTTAgentCommandInfo_t xCommandParams =
    {
        .blockTimeMs                 = MQTT_PUBLISH_BLOCK_TIME_MS,
        .cmdCompleteCallback         = prvPublishCommandCallback,
        .pCmdCompleteCallbackContext = ( void * ) xTaskGetCurrentTaskHandle(),
    };

    if( xPublishInfo.qos > MQTTQoS0 )
    {
        xCommandParams.pCmdCompleteCallbackContext = ( void * ) xTaskGetCurrentTaskHandle();
    }

    /* Clear the notification index */
    xTaskNotifyStateClearIndexed( NULL, MQTT_NOTIFY_IDX );


    xStatus = MQTTAgent_Publish( xAgentHandle,
                                 &xPublishInfo,
                                 &xCommandParams );

    if( xStatus == MQTTSuccess )
    {
        uint32_t ulNotifyValue = 0;
        BaseType_t xResult = pdFALSE;

        xResult = xTaskNotifyWaitIndexed( MQTT_NOTIFY_IDX,
                                          0xFFFFFFFF,
                                          0xFFFFFFFF,
                                          &ulNotifyValue,
                                          pdMS_TO_TICKS( MQTT_PUBLISH_NOTIFICATION_WAIT_MS ) );

        if( xResult )
        {
            xStatus = ( MQTTStatus_t ) ulNotifyValue;

            if( xStatus != MQTTSuccess )
            {
                LogError( "MQTT Agent returned error code: %d during publish operation.",
                          xStatus );
                xResult = pdFALSE;
            }
        }
        else
        {
            LogError( "Timed out while waiting for publish ACK or Sent event. xTimeout = %d",
                      pdMS_TO_TICKS( MQTT_PUBLISH_NOTIFICATION_WAIT_MS ) );
            xResult = pdFALSE;
        }
    }
    else
    {
        LogError( "MQTTAgent_Publish returned error code: %d.", xStatus );
    }

    return( xStatus == MQTTSuccess );
}


/* CRC init function */
static void CRC_Init(void)
{
  CRC_HandleTypeDef hcrc;
  hcrc.Instance                     = CRC;
  hcrc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;
  __HAL_RCC_CRC_CLK_ENABLE();
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
      LogError( "CRC Init Error" );
  }
}

/*-----------------------------------------------------------*/
static BaseType_t xInitSensors( void )
{
    int32_t lBspError = BSP_ERROR_NONE;

      //BSP_COM_Init(0U);

      status = CUSTOM_RANGING_SENSOR_Init(CUSTOM_VL53L5CX);

      if (status != BSP_ERROR_NONE)
      {
        printf("CUSTOM_RANGING_SENSOR_Init failed\n");
        while(1);
      }
      else {return lBspError;}
}

/*-----------------------------------------------------------*/
void vMotionSensorsPublish( void * pvParameters )
{
    ( void ) pvParameters;
    BaseType_t xResult = pdFALSE;
    BaseType_t xExitFlag = pdFALSE;

    MQTTAgentHandle_t xAgentHandle = NULL;
    char pcPayloadBuf[ MQTT_PUBLISH_MAX_LEN ];
    char pcTopicString[ MQTT_PUBLICH_TOPIC_STR_LEN ] = { 0 };
    char * pcDeviceId = NULL;
    int lTopicLen = 0;

    /* Configure the system clock */

    //SystemClock_Config();

    //CRC initialization
    CRC_Init();

    //sensor init
    xResult = xInitSensors();


    /* Initialize the Neural Network library  */
     Network_Init();



    pcDeviceId = KVStore_getStringHeap( CS_CORE_THING_NAME, NULL );

    if( pcDeviceId == NULL )
    {
        xExitFlag = pdTRUE;
    }
    else
    {
        lTopicLen = snprintf( pcTopicString, ( size_t ) MQTT_PUBLICH_TOPIC_STR_LEN, "%s/motion_sensor_data", pcDeviceId );
    }

    if( ( lTopicLen <= 0 ) || ( lTopicLen > MQTT_PUBLICH_TOPIC_STR_LEN ) )
    {
        LogError( "Error while constructing topic string." );
        xExitFlag = pdTRUE;
    }

    vSleepUntilMQTTAgentReady();

    xAgentHandle = xGetMqttAgentHandle();


    while( xExitFlag == pdFALSE )
    {
        /* Interpret sensor data */
        int32_t lBspError = BSP_ERROR_NONE;

 //todo: add wait notify from isr callback


        /* Wait for available ranging data */
        lBspError = MX_VL53L5CX_SimpleRanging_Process();




        // Pre-process data
		Network_Preprocess(&App_Config);

		//print_result(&App_Config);
	//	LogInfo( " -------------  %f     --------",App_Config.HANDPOSTURE_Input_Data.ranging[27]);
		LogInfo( " ------------- VERSION  2   ...........................................................     --------");

			/* Run inference */

			 Network_Inference(&App_Config);
			 /* Run Post processing  */
			 Network_Postprocess(&App_Config);



        if( lBspError == BSP_ERROR_NONE )
        {

        	int lbytesWritten = snprintf( pcPayloadBuf,MQTT_PUBLISH_MAX_LEN,  "{"

        			"\"pred\": %d"
        			","
					"\"dist\": %.1f"
                    "}",(App_Config.AI_Data.handposture_label),App_Config.HANDPOSTURE_Input_Data.ranging[27]/10.0);

			//classe_table[(App_Config.AI_Data.handposture_label)],App_Config.HANDPOSTURE_Input_Data.ranging[27]

        		LogError( "distance -------------  %d     --------",App_Config.AI_Data.model_output);

		    	LogInfo("\033[1;32m The output \033[0m : %d \r\n",(App_Config.AI_Data.handposture_label));




            if( ( lbytesWritten < MQTT_PUBLISH_MAX_LEN  )  && ( xIsMqttAgentConnected() == pdTRUE )  )
            {
                xResult = prvPublishAndWaitForAck( xAgentHandle,
                                                   pcTopicString,
                                                   pcPayloadBuf,
                                                   ( size_t ) lbytesWritten );

                if( xResult != pdPASS )
                {
                    LogError( "Failed to publish motion sensor data" );
                }
            }
        }
     //   vTaskDelay( pdMS_TO_TICKS( MQTT_PUBLISH_PERIOD_MS ) );
    }

    vPortFree( pcDeviceId );
}


void print_result(AppConfig_TypeDef *   conf)
{
  int8_t i, j, k, l;
  uint8_t zones_per_line;
  uint8_t number_of_zones;
  zones_per_line = 8;
  number_of_zones = 64;

  /* clear screen */
  printf("%c[2H", 27);

  printf("Demo of Hand-posture recognition model with time-of-flight sensor application\r\n");

  printf("----------------------------------------\r\n\r\n");


  printf("\r\n");

  printf("Cell Format :\r\n\r\n");
  for (l = 0; l < RANGING_SENSOR_NB_TARGET_PER_ZONE; l++)
  {
	  printf(" \033[38;5;10m%20s\033[0m : %20s\n", "Distance [mm]", "Status");

	  printf(" %20s : %20s\r\n", "Signal [kcps/spad]", "Ambient [kcps/spad]");

  }

  printf("\r\n");

  for (j = 0; j <number_of_zones ; j += zones_per_line)
  {
    for (i = 0; i < zones_per_line; i++) /* number of zones per line */
    	printf(" -----------------");
    printf("\r\n");

    for (i = 0; i < zones_per_line; i++)
    	printf("|                 ");
    printf("|\r\n");

    for (l = 0; l < RANGING_SENSOR_NB_TARGET_PER_ZONE; l++)
    {
      /* Print distance and status */
      for (k = (zones_per_line - 1); k >= 0; k--)
      {
        if (conf->RangingData.nb_target_detected[j+k]  > 0  )
        {
          if (conf->HANDPOSTURE_Input_Data.ranging[j+k] < 150)
          {
        	  printf("| \033[38;5;9m%f\033[0m ",
            		conf->HANDPOSTURE_Input_Data.ranging[j+k]);

          } else if (conf->HANDPOSTURE_Input_Data.ranging[j+k] > 1000)
          {
        	  printf("| \033[38;5;9m%f\033[0m   ",
            		conf->HANDPOSTURE_Input_Data.ranging[j+k]);
          }
          else{
        	  printf("| \033[38;5;10m%f\033[0m  ",
            		  conf->HANDPOSTURE_Input_Data.ranging[j+k]);
          }
        }
        else
        	printf("| %5s  :  %5s ", "X", "X");
      }
      printf("|\r\n");


        /* Print Signal and Ambient */
        for (k = (zones_per_line - 1); k >= 0; k--)
        {
          if (conf->RangingData.nb_target_detected[j+k] > 0)
          {
        	  printf("| %f  :  ", conf->HANDPOSTURE_Input_Data.peak[j+k]);

          }
          else
        	  printf("| %5s  :  %5s ", "X", "X");
        }
        printf("|\r\n");

    }
  }

  for (i = 0; i < zones_per_line; i++)
	  printf(" -----------------");
  printf("\r\n");

}
