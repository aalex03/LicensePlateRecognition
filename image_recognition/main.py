from src import ocr
#from src import camera as cam
from PIL import Image
from src import validator
#image = cam.Camera.capture()
#image = Image.open(".temp/nr.png")
image = Image.open("data/input.jpg")
licensePlates = []
licensePlates.append(ocr.LicensePlateProcessor.process(image,"canny",False))
#licensePlates.append(ocr.LicensePlateProcessor.process(image,"haar",True))

for plate in licensePlates:
    stripped_plate = plate.replace(" ","")
    if validator.LicensePlateValidator.validate(stripped_plate):
        print("Found valid license plate: " + stripped_plate)
        break
exit(0)