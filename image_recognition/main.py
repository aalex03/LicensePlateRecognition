from src import ocr
#from src import camera as cam
from PIL import Image
from src import validator
#image = cam.Camera.capture()
#image = Image.open(".temp/nr.png")
image = Image.open("data/logan_side.jpg")
licensePlates = ocr.LicensePlateProcessor.process(image,"haar",True)
for plate in licensePlates:
    if validator.LicensePlateValidator.validate(plate):
        print("Found valid license plate: " + plate)
        break
exit(0)