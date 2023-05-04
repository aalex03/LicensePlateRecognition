import cv2

import imutils 

import numpy as np

import pytesseract

import validator as vali

from PIL import Image

f = open("test_runs.txt", "w")
psm_val=6 #default
class LicensePlateProcessor:
    def __init__(self) -> None:
        pass

    @staticmethod
    def process(image: Image) -> str:

        image_np = np.array(image)  # conversion from PIL image to cv2 image
        img = cv2.cvtColor(image_np, cv2.COLOR_RGB2BGR)
        #cv2.imshow("", img)
        cv2.waitKey()
        img = cv2.resize(img, (620, 480))

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)  # convert to grey scale

        gray = cv2.bilateralFilter(gray, 11, 17, 17)  # Blur to reduce noise

        edged = cv2.Canny(gray, 30, 200)  # Perform Edge detection

        # find contours in the edged image, keep only the largest

        # ones, and initialize our screen contour

        cnts = cv2.findContours(
            edged.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        cnts = imutils.grab_contours(cnts)

        cnts = sorted(cnts, key=cv2.contourArea, reverse=True)[:10]

        screenCnt = None

        # loop over our contours

        for c in cnts:

            # approximate the contour

            peri = cv2.arcLength(c, True)

            approx = cv2.approxPolyDP(c, 0.018 * peri, True)

        # if our approximated contour has four points, then

        # we can assume that we have found our screen

            if len(approx) == 4:

                screenCnt = approx

                break

        if screenCnt is None:

            detected = 0

            print("No contour detected")

            return None

        else:
            detected = 1

        if detected == 1:

            cv2.drawContours(img, [screenCnt], -1, (0, 255, 0), 3)

        # Masking the part other than the number plate

        mask = np.zeros(gray.shape, np.uint8)

        new_image = cv2.drawContours(mask, [screenCnt], 0, 255, -1,)

        new_image = cv2.bitwise_and(img, img, mask=mask)

        # Now crop

        (x, y) = np.where(mask == 255)

        (topx, topy) = (np.min(x), np.min(y))

        (bottomx, bottomy) = (np.max(x), np.max(y))

        Cropped = gray[topx:bottomx+1, topy:bottomy+1]
        
        (thresh, Cropped )= cv2.threshold( Cropped, 127, 255, cv2.THRESH_BINARY)
        
        Cropped = ~Cropped

        # Read the number plate
        
        
        text =str(pytesseract.image_to_string(Cropped, config=f'--psm {psm_val} -c tessedit_char_whitelist=0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'))
        f.write(f'  Detected text is:{text}\n')
        f.write("   " + vali.LicensePlateValidator.algo(text))
        """
        cv2.imshow('image', img)

        cv2.imshow('Cropped', Cropped)

        cv2.waitKey(0)

        cv2.destroyAllWindows()
        """
        return text
"""
test_img=Image.open("D:\AC_LABS_2023_VEONEER\python_workspace\LicensePlateRecognition\data\logan_front.jpg")
pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'
LicensePlateProcessor.process(test_img)
"""
for psm_val in [6,7,8,11,12,13]:
    f.write(f'Test pentru valoarea psm {psm_val}\n')
    for photo_index in range(9,27):
        f.write(f'  Test pentru poza p{photo_index}.jpg\n')
        test_img=Image.open(f'D:\AC_LABS_2023_VEONEER\python_workspace\LicensePlateRecognition\data\p{photo_index}.jpg')
        pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'
        LicensePlateProcessor.process(test_img)
        f.write("\n\n")
    print(f'Incheiat test pentru valoarea psm {psm_val}\n')

f.close()


