from src import ocr
from src import camera as cam
from PIL import Image
from src import validator
import sys

if(len(sys.argv)>1):
    image = Image.open(sys.argv[1])
else:
    image = cam.Camera.capture()

licensePlates = ocr.LicensePlateProcessor.process(image,"canny",True)

if(len(licensePlates)==0):
    licensePlates.append(ocr.LicensePlateProcessor.process(image,"haar",True))
if(len(licensePlates)==0):
    print("No license plate found")
    exit(1)
    
for plate in licensePlates:
    stripped_plate = plate.replace(" ","")
    if validator.LicensePlateValidator.validate(stripped_plate):
        print(f"{stripped_plate.strip()}")
        break
exit(0)