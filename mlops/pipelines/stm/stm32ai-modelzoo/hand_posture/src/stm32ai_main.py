# /*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2022-2023 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/
import os
import sys
from hydra.core.hydra_config import HydraConfig
import hydra
import warnings

warnings.filterwarnings("ignore")
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3' 

import tensorflow as tf
from omegaconf import DictConfig, OmegaConf
import mlflow
import argparse
import sys
import json
import pathlib
import boto3
from sagemaker.experiments.run import load_run
from sagemaker.experiments.run import Run
from sagemaker.session import Session
import shutil
import glob
import subprocess


sys.path.append(os.path.join(os.path.dirname(__file__), '/opt/ml/processing/outputs/build'))
sys.path.append(os.path.join(os.path.dirname(__file__), '/opt/ml/processing/input/model/outputs/saved_models'))
sys.path.append(os.path.join(os.path.dirname(__file__), '/opt/ml/processing/input'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/benchmarking'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/deployment'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/quantization'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/optimization'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/evaluation'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/training'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/utils'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../deployment'))
sys.path.append(os.path.join(os.path.dirname(__file__), './data_augmentation'))
sys.path.append(os.path.join(os.path.dirname(__file__), './preprocessing'))
sys.path.append(os.path.join(os.path.dirname(__file__), './training'))
sys.path.append(os.path.join(os.path.dirname(__file__), './utils'))
sys.path.append(os.path.join(os.path.dirname(__file__), './evaluation'))
sys.path.append(os.path.join(os.path.dirname(__file__), './models'))

if sys.platform.startswith('linux'):
    sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/optimization/linux'))
elif sys.platform.startswith('win'):
    sys.path.append(os.path.join(os.path.dirname(__file__), '../../common/optimization/windows'))

from logs_utils import mlflow_ini, log_to_file
from gpu_utils import set_gpu_memory_limit
from cfg_utils import get_random_seed
from preprocess import preprocess
from visualize_utils import display_figures
from parse_config import get_config
from trainn import train
from evaluatee import evaluate
from deploy import deploy
from common_benchmark import benchmark
from typing import Optional

session = Session(boto3.session.Session(region_name="eu-central-1"))
def process_mode(mode: str = None, 
                 configs: DictConfig = None, 
                 train_ds: tf.data.Dataset = None,
                 valid_ds: tf.data.Dataset = None,
                 test_ds: tf.data.Dataset = None, 
                 float_model_path: Optional[str] = None,
                 fake: Optional[bool] = False) -> None:
    """
    Process the selected mode of operation.

    Args:
        mode (str): The selected mode of operation. Must be one of 'train', 'evaluate', or 'predict'.
        configs (DictConfig): The configuration object.
        train_ds (tf.data.Dataset): The training dataset. Required if mode is 'train'.
        valid_ds (tf.data.Dataset): The validation dataset. Required if mode is 'train' or 'evaluate'.
        test_ds (tf.data.Dataset): The test dataset. Required if mode is 'evaluate' or 'predict'.
        float_model_path(str, optional): Model path . Defaults to None
        fake (bool, optional): Whether to use fake data for representative dataset generation. Defaults to False.
    Returns:
        None
    Raises:
        ValueError: If an invalid operation_mode is selected or if required datasets are missing.
    """

    # Check the selected mode and perform the corresponding operation
    # logging the operation_mode in the output_dir/stm32ai_main.log file
    log_to_file(configs.output_dir, f'operation_mode: {mode}')
    if mode == 'training':
        if test_ds:
            train(cfg=configs, train_ds=train_ds, valid_ds=valid_ds, test_ds=test_ds)
        else:
            train(cfg=configs, train_ds=train_ds, valid_ds=valid_ds)
        display_figures(configs)
        print('[INFO] : Training complete.')
        
    elif mode == 'evaluation':
        print('[INFO] : Starting evaluation .')
        if test_ds:
           report_dict = evaluate(cfg=configs, eval_ds=test_ds, name_ds="test set")
        else:
           report_dict = evaluate(cfg=configs, eval_ds=valid_ds, name_ds="validation set")
        display_figures(configs)
         # log to SageMaker experiments
        with load_run(experiment_name=os.environ['EXPERIMENT_NAME'], run_name=os.environ['RUN_NAME'], sagemaker_session=session) as run:
           # Log a metrics over the course of a run
           run.log_metric(name="test_acc", value=report_dict['multiclass_classification_metrics']['test_acc']['value'])
        print('[INFO] : Evaluation complete.')
    elif mode == 'deployment':
        deploy(cfg=configs)
        print('[INFO] : Deployment complete.')
    elif mode == 'benchmarking':
        benchmark(cfg=configs)
        
        print('[INFO] : Benchmark complete.')

    # Raise an error if an invalid mode is selected
    else:
        raise ValueError(f"Invalid mode: {mode}")
    
   
        
    # Record the whole hydra working directory to get all info
    mlflow.log_artifact(configs.output_dir)   
    if mode in ['benchmarking']: #'chain_qb', 'chain_eqeb', 'chain_tbqeb'
        mlflow.log_param("model_path", configs.general.model_path)
        mlflow.log_param("stm32ai_version", configs.tools.stm32ai.version)
        mlflow.log_param("target", configs.benchmarking.board)
    
    if mode == 'evaluation':    
        #Create a PropertyFile
        # A PropertyFile is used to reference outputs from a processing step, in order to use in a condition step
        output_dir = "/opt/ml/processing/evaluation"  
        pathlib.Path(output_dir).mkdir(parents=True, exist_ok=True)
        evaluation_path = f"{output_dir}/evaluation.json"
        with open(evaluation_path, "w") as f:
            f.write(json.dumps(report_dict))
            print("Done creating evaluation.json")
    # logging the completion of the chain
    log_to_file(configs.output_dir, f'operation finished: {mode}')


@hydra.main(version_base=None, config_path="", config_name="user_config")
def main(cfg: DictConfig) -> None:
    """
    Main entry point of the script.

    Args:
        cfg: Configuration dictionary.

    Returns:
        None
    """

    # Configure the GPU (the 'general' section may be missing)
    if "general" in cfg and cfg.general:
        # Set upper limit on usable GPU memory
        if "gpu_memory_limit" in cfg.general and cfg.general.gpu_memory_limit:
            set_gpu_memory_limit(cfg.general.gpu_memory_limit)
            print(f"[INFO] : Setting upper limit of usable GPU memory to {int(cfg.general.gpu_memory_limit)}GBytes.")
        else:
            print("[WARNING] The usable GPU memory is unlimited.\n"
                  "Please consider setting the 'gpu_memory_limit' attribute "
                  "in the 'general' section of your configuration file.")
    
    op = cfg.operation_mode
    if op == 'evaluation' and cfg.general.model_path is None:
        
        #unT the folder that conatins the h5 model
        shutil.unpack_archive('/opt/ml/processing/input/model/model.tar.gz', '/opt/ml/processing/input/model/')
        subprocess.run(["pwd"]) 
        subprocess.run(["ls","/opt/ml/processing/input/model/"]) 
        model_path = glob.glob('/opt/ml/processing/input/model/**/**/**/best_model.h5')
        print(f"model path found for evaluation: {model_path[0]} ")
        cfg.general.model_path = model_path[0]
        cfg.dataset.training_path = ""
        cfg.dataset.test_path = "/opt/ml/processing/input/datasets/ST_VL53L8CX_handposture_dataset"
       

    
    elif op == 'training':
        cfg.dataset.training_path = "/opt/ml/input/data/train/datasets/ST_VL53L8CX_handposture_dataset"
      
    
    elif op == 'benchmarking':
        model_path = glob.glob('/opt/ml/processing/input/model/**/**/**/best_model.h5')
        print(f"model path found for {op}: {model_path[0]} ")
        cfg.general.model_path = model_path[0]
        
        #cfg.tools.stedgeai.path_to_stedgeai = "C:/STMicroelectronics/STM32Cube/Repository/Packs/STMicroelectronics/X-CUBE-AI/8.1.0/Utilities/windows/stm32ai.exe"
        #cfg.tools.path_to_cubeIDE =  "C:/ST/STM32CubeIDE_1.15.0/STM32CubeIDE/stm32cubeide.exe" 
    else:
      
        print("Le chemin du modèle n'a pas été défini dans la configuration.(propablement a training)")
    
    
    # Parse the configuration file
    cfg = get_config(cfg) 
    
    cfg.output_dir = HydraConfig.get().run.dir    
    mlflow_ini(cfg)

    # Seed global seed for random generators
    seed = get_random_seed(cfg)
    print(f'[INFO] : The random seed for this simulation is {seed}')
    if seed is not None:
        tf.keras.utils.set_random_seed(seed)

    # Extract the mode from the command-line arguments
    mode = cfg.operation_mode
    valid_modes = ['training', 'evaluation']
    if mode in valid_modes:
        # Perform further processing based on the selected mode
        preprocess_output = preprocess(cfg=cfg)
        train_ds, valid_ds, test_ds = preprocess_output
        # Process the selected mode
        process_mode(mode=mode, 
                     configs=cfg, 
                     train_ds=train_ds, 
                     valid_ds=valid_ds,
                     test_ds=test_ds)
    else:
        # Process the selected mode
        process_mode(mode=mode, 
                     configs=cfg)
    
   
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--config-path', type=str, default='', help='Path to folder containing configuration file')
    parser.add_argument('--config-name', type=str, default='user_config', help='name of the configuration file')
    # add arguments to the parser
    parser.add_argument('params', nargs='*',
                        help='List of parameters to over-ride in config.yaml')
    args = parser.parse_args()

    # Call the main function
    main()

    # log the config_path and config_name parameters
    mlflow.log_param('config_path', args.config_path)
    mlflow.log_param('config_name', args.config_name)
    mlflow.end_run()
