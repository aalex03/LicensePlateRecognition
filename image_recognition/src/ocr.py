import cv2

import imutils

import numpy as np

import pytesseract

from PIL import Image

class LicensePlateProcessor:
    images = []
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
    def _threshold(image):
        image = cv2.adaptiveThreshold(image,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY,11,2)
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
    def _saveImages(images):
        for (name,image) in images:
            cv2.imwrite(f".temp/{name}.png",image)

    @staticmethod
    def _mask_and_crop(img,contour):
        mask = np.zeros(img.shape[:2],np.uint8)

        new_image = cv2.drawContours(mask,[contour],0,255,-1,)

        new_image = cv2.bitwise_and(img,img,mask=mask)

        # Now crop

        (x, y) = np.where(mask == 255) 

        (topx, topy) = (np.min(x), np.min(y))

        (bottomx, bottomy) = (np.max(x), np.max(y))

        cropped = new_image[topx:bottomx+1, topy:bottomy+1]

        return cropped

    @staticmethod
    def deskew_image(image):
        # Convert the image to grayscale
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # Apply adaptive thresholding to create a binary image
        _, binary = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
        
        # Find contours in the binary image
        contours, _ = cv2.findContours(binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        # Find the largest contour (presumably the object of interest)
        largest_contour = max(contours, key=cv2.contourArea)
        
        # Determine the orientation angle of the contour using the minimum area rectangle
        _, _, angle = cv2.minAreaRect(largest_contour)
        
        # Rotate the image to deskew it
        rows, cols = image.shape[:2]
        rotation_matrix = cv2.getRotationMatrix2D((cols/2, rows/2), angle, 1)
        deskewed_image = cv2.warpAffine(image, rotation_matrix, (cols, rows), flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REPLICATE)
        
        return deskewed_image
    
    @staticmethod
    def _haar_cascade(image,imgArray):
        plate_cascade = cv2.CascadeClassifier("data/haarcascade_russian_plate_number.xml")
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        imgArray.append(("gray",gray))
        plates = plate_cascade.detectMultiScale(gray, 1.3, 5)
        contours = []
        for (x, y, w, h) in plates:
            contour_points = np.array([(x, y), (x, y + h), (x + w, y + h), (x + w, y)])
            contours.append(contour_points)

        cv2.drawContours(image, contours, -1, (0, 255, 0), 2)
        return contours
    
    @staticmethod
    def _canny_detection(image,imgArray):
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY) #convert to grey scale
        imgArray.append(("gray",gray))
        blur = LicensePlateProcessor._blur(gray,"bilateral",(11,17,17)) #Blur to reduce noise
        imgArray.append(("blur",blur))
        edged = cv2.Canny(blur, 30, 200) #Perform Edge detection
        contours = LicensePlateProcessor._findContours(edged)
        imgArray.append(("edged",edged))
        cv2.drawContours(image, contours, -1, (0, 255, 0), 3)
        return contours
    
    @staticmethod
    def process(image : Image, option : str, saveImages : bool) -> list:
        
        img = LicensePlateProcessor._convertPILtoCV2(image)
        images = []
        plates = []
        if option == "canny":
            contours = LicensePlateProcessor._canny_detection(img,images)
        elif option == "haar":
            contours = LicensePlateProcessor._haar_cascade(img,images)
        i = 0
        if len(contours) == 0:
            print("no license plate found")
            return [""]
        for c in contours:
            i = i+1
            cropped = LicensePlateProcessor._mask_and_crop(img,c)
            cropped = cv2.bitwise_not(cropped)
            images.append((f"cropped{i}",cropped))
            for psm_val in [1,3,4,5,6,7,8,9,10,11,12,13]:
                text = pytesseract.image_to_string(cropped, config=f'-c tessedit_char_whitelist=QWERTYUIOPASDFGHJKLZXCVBNM1234567890 --psm {psm_val}')
                if text.strip() != "":
                    print(f"Text for crop {i}: {text.strip()} at psm {psm_val}")
                    plates.append(text.strip())
        if saveImages == True:
            print("Saving images")
            LicensePlateProcessor._saveImages(images)
        return plates