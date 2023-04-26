from src import ocr
from src import camera as cam
from PIL import Image
from src import validator
#image = cam.Camera.capture()
image = Image.open(".temp/nr.png")
licensePlate = ocr.LicensePlateProcessor.process(image)