import subprocess
import sys

image_recognition_path = "image_recognition/main.py"
if(sys.argv>1):
    subprocess.run(["python",image_recognition_path,sys.argv[1]])
else:
    subprocess.run(["python",image_recognition_path])