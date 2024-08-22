import sys,os,logging
import json
import argparse
import subprocess
import traceback
import yaml
from pathlib import Path

if __name__ == "__main__":
    
    module_path = os.path.abspath('hand_posture/src')
    if module_path not in sys.path:
        sys.path.append(module_path)
        
    try:
        chemin_fichier = Path("hand_posture/src/stm32ai_main.py")
        # Ouvrir et lire le fichier YAML
        with open(chemin_fichier, "r") as fichier:
            donnees = yaml.safe_load(fichier)
            
        # Afficher le contenu original
        print("Contenu original:")
        print(yaml.dump(donnees, default_flow_style=False))
        
        
        # Modifier hydra.run.dir
        if 'hydra' in donnees and 'run' in donnees['hydra']:
            donnees['hydra']['run']['dir'] = '/opt/ml/processing/outputs/build'
        else:
            print("L'attribut 'hydra.run.dir' n'existe pas dans le fichier YAML.")
        
        with open(chemin_fichier, "w") as fichier:
            yaml.dump(donnees, fichier, default_flow_style=False)
        
        subprocess.run([
        "python",
        os.path.join(module_path, "stm32ai_main.py"),"operation_mode='evaluation'"])
        #   subprocess.run([
        #  "python",
        # os.path.join(module_path, "stm32ai_main.py"),"operation_mode='benchmark'"])
        
        
        print('Evaluation complete.')

            # A zero exit code causes the job to be marked a Succeeded.
        sys.exit(0)
    except Exception as e:
        # Write out an error file. This will be returned as the failureReason in the
        # DescribeTrainingJob result.
        trc = traceback.format_exc()
        # Printing this causes the exception to be in the training job logs, as well.
        print('Exception during evaluation: ' + str(e) + '\n' + trc, file=sys.stderr)
        # A non-zero exit code causes the training job to be marked as Failed.
        sys.exit(255)