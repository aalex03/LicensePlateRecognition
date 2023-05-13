from src import ocr
from src import camera as cam
from PIL import Image
from src import validator
#image = cam.Camera.capture()
#image = Image.open(".temp/nr.png")
image = Image.open("data/logan_front.jpg")
licensePlate = ocr.LicensePlateProcessor.process(image)
print(f"License plate \"{licensePlate}\" is valid: {validator.LicensePlateValidator.validate(licensePlate)}")
exit(0)