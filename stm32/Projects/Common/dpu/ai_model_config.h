/********************************************************************************
* @file    ai_model_config.h
* @author  STMicroelectronics - AIS - MCD Team
* @version 1.0.0
* @date    2023-Nov-17
* @brief   Configure the getting started functionality
*
*  Each logic module of the application should define a DEBUG control byte
* used to turn on/off the log for the module.
*
*********************************************************************************
* @attention
*
* Copyright (c) 2022 STMicroelectronics
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file in
* the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*********************************************************************************
*/
/* ---------------    Generated code    ----------------- */
#ifndef AI_MODEL_CONFIG_H
#define AI_MODEL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define CTRL_X_CUBE_AI_MODE_NAME                 "X-CUBE-AI TOF"
#define NB_CLASSES          (8)
#define CTRL_X_CUBE_AI_MODE_NB_OUTPUT            (1U)

#define INPUT_HEIGHT        (8)
#define INPUT_WIDTH         (8)
#define INPUT_CHANNELS      (2)

#define CLASSES_TABLE const char* classes_table[NB_CLASSES] = {"None" ,   "FlatHand" ,   "Like" ,   "Love" ,   "Dislike" ,   "BreakTime" , "CrossHands" ,   "Fist"};

#define EVK_LABEL_TABLE const int evk_label_table[NB_CLASSES] = {\
   0 ,   20 ,   21 ,   24 ,   25 ,   27 ,\
   28 ,   32};\

#define BACKGROUND_REMOVAL (120)
#define MAX_DISTANCE (350)
#define MIN_DISTANCE (150)

#ifdef __cplusplus
}
#endif

#endif /* AI_MODEL_CONFIG_H */
