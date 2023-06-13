from src import ocr
from src import camera as cam
from PIL import Image
from src import validator
image = cam.Camera.capture()
#image = Image.open(".temp/nr.png")
#mage = Image.open("data/input.jpg")
licensePlates = ocr.LicensePlateProcessor.process(image,"canny",True)
#licensePlates.append(ocr.LicensePlateProcessor.process(image,"haar",True))

for plate in licensePlates:
    stripped_plate = plate.replace(" ","")
    if validator.LicensePlateValidator.validate(stripped_plate):
        print(f"{stripped_plate.strip()}")
        break
exit(0)