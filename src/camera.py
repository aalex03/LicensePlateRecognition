import picamera
from io import BytesIO
from time import sleep
from PIL import Image


class Camera:
    @staticmethod
    def capture() -> Image:
        camera = picamera.PiCamera()
        stream = BytesIO()
        sleep(4)
        camera.capture(stream,format="jpeg")
        image = Image.open(stream)
        return image

