from src import ocr
from src import camera as cam
from PIL import Image
from src import validator
import cv2
import sys

if(len(sys.argv)>1):
    image = Image.open(sys.argv[1])
else:
    image = cam.Camera.capture()

licensePlates = ocr.LicensePlateProcessor.process(image,"canny",True)
validPlates = []
for plate in licensePlates:
    stripped_plate = plate.replace(" ","")
    if validator.LicensePlateValidator.validate(stripped_plate):
        if(stripped_plate not in validPlates):
            validPlates.append(stripped_plate.strip())
        
if(len(validPlates) == 0):
    licensePlates = ocr.LicensePlateProcessor.process(image,"haar",True)
    for plate in licensePlates:
        stripped_plate = plate.replace(" ","")
        if validator.LicensePlateValidator.validate(stripped_plate):
            if(stripped_plate not in validPlates):
                validPlates.append(stripped_plate.strip())

if len(validPlates) > 0:
    for plate in validPlates:
            print(plate)
exit(1)