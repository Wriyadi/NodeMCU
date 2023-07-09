import cv2
import pyautogui
import numpy as np

while True:
    image = pyautogui.screenshot(region = (398, 210, 165, 143))
    image = cv2.cvtColor(np.array(image), cv2.COLOR_RGB2BGR)

    count_black_pixel=np.sum(image < 100)
    print('Number of Black Pixel', count_black_pixel)

    if count_black_pixel > 2950 and count_black_pixel <30000 :
        pyautogui.press('up')
    
    cv2.imshow("image", image)
    if cv2.waitKey(25) & 0xff == ord('q'):
        cv2.destroyAllWindows()
        break
