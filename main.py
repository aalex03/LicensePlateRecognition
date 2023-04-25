from src import ocr
from src import camera as cam
from PIL import Image
image = cam.Camera.capture()
#image = Image.open("data/logan_front.jpg")
licensePlate = ocr.LicensePlateProcessor.process(image)