import cv2

import imutils

import numpy as np

import pytesseract

from PIL import Image

class LicensePlateProcessor:
    def __init__(self) -> None:
        pass

    @staticmethod
    def _blur(image, type : str, settings):
        if type == "bilateral":
            image = cv2.bilateralFilter(image, settings[0],settings[1],settings[2])
        else:
            if type == "gaussian":
                image = cv2.GaussianBlur(image,(settings[0],settings[1]),settings[3])
        return image

        

    @staticmethod
    def _convertPILtoCV2(image : Image):
        image_np = np.array(image) #conversion from PIL image to cv2 image
        img = cv2.cvtColor(image_np,cv2.COLOR_RGB2BGR)
        return img

    @staticmethod
    def _findContours(edged) -> list:
        cnts = cv2.findContours(edged.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        cnts = imutils.grab_contours(cnts)
        cnts = sorted(cnts, key = cv2.contourArea, reverse = True)[:10]
        result = []
        for c in cnts:

        # approximate the contour

            peri = cv2.arcLength(c, True)

            approx = cv2.approxPolyDP(c, 0.018 * peri, True)

        

        # if our approximated contour has four points, then

        # we can assume that we have found our screen

            if len(approx) == 4:

                result.append(approx)
        
        return result

    @staticmethod
    def _boxes(img):
        h, w, _ = img.shape # assumes color image

        # run tesseract, returning the bounding boxes
        boxes = pytesseract.image_to_boxes(img) # also include any config options you use

        # draw the bounding boxes on the image
        for b in boxes.splitlines():
            b = b.split(' ')
            img = cv2.rectangle(img, (int(b[1]), h - int(b[2])), (int(b[3]), h - int(b[4])), (0, 255, 0), 2)

        # show annotated image and wait for keypress
        cv2.imshow("filename", img)
        cv2.waitKey(0)

    @staticmethod
    def _mask_and_crop(img,contour):

        mask = np.zeros(img.shape,np.uint8)

        new_image = cv2.drawContours(mask,[contour],0,255,-1,)

        new_image = cv2.bitwise_and(img,img,mask=mask)

        cv2.imshow("masked",new_image)

        # Now crop

        (x, y) = np.where(mask == 255) 

        (topx, topy) = (np.min(x), np.min(y))

        (bottomx, bottomy) = (np.max(x), np.max(y))

        cropped = new_image[topx:bottomx+1, topy:bottomy+1]

        return cropped

    @staticmethod
    def process(image : Image) -> str:
        
        img = LicensePlateProcessor._convertPILtoCV2(image)
        
        img = cv2.resize(img, (620,480) )

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY) #convert to grey scale


        gray = LicensePlateProcessor._blur(gray,"bilateral",(11,17,17)) #Blur to reduce noise


        cv2.imshow("",gray)
        cv2.waitKey()

        #(_ , gray) = cv2.threshold(gray,0,255,cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU) 
        #gray = cv2.GaussianBlur(gray,(3,3),0)
        #gray= cv2.equalizeHist(gray) #Increase contrast

        edged = cv2.Canny(gray, 30, 200) #Perform Edge detection

        contours = LicensePlateProcessor._findContours(edged)
        
        print(len(contours))

        cv2.drawContours(img, contours, -1, (0, 255, 0), 3)

        cv2.imshow("contoured",img)
        cv2.waitKey()
        i = 0
        for c in contours:
            cropped = LicensePlateProcessor._mask_and_crop(gray,c)
            text = pytesseract.image_to_string(cropped, config='-c tessedit_char_whitelist=QWERTYUIPASDFGHJKLZXCVBNM1234567890 --psm 11')
            print(f"text for crop {i}: {text}")
            cv2.imshow(f"crop {i}",cropped)
            i = i+1
            cv2.waitKey()

        cv2.destroyAllWindows()
        
        return text