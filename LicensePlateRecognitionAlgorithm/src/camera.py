import picamera
from io import BytesIO
from time import sleep
from PIL import Image


class Camera:
    @staticmethod
    def capture() -> Image:
        camera = picamera.PiCamera()
        stream = BytesIO()
        sleep(5)
        camera.capture(stream,format="png")
        image = Image.open(stream)
        image = image.convert("RGB")
        return image

