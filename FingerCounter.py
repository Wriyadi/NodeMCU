import cv2
from cvzone.HandTrackingModule import HandDetector

# Initialize the hand detector with a maximum of 2 hands
detector = HandDetector(maxHands=2, detectionCon=0.8)

# Start capturing video from the webcam
video = cv2.VideoCapture(0)

while True:
    _, img = video.read()
    img = cv2.flip(img, 1)  # Flip the image for a mirror effect

    # Detect hands and retrieve the list of detected hands
    hands, img = detector.findHands(img, draw=True)  # Get hands and processed image
    total_fingers = 0  # Initialize total fingers count

    # Check if hands are detected
    if hands:
        for hand in hands:
            # Use the hand data directly in fingersUp()
            finger_up = detector.fingersUp(hand)  
            total_fingers += sum(finger_up)  # Sum raised fingers for total

    # Display the total fingers on the video feed
    cv2.putText(img, f'Total Fingers: {total_fingers}', (50, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    # Show the video with the total fingers count
    cv2.imshow("Video", img)

    # Break the loop if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release video capture and close windows
video.release()
cv2.destroyAllWindows()
