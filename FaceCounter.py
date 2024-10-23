import cv2
import mediapipe as mp

# Initialize MediaPipe Face Detection and drawing utilities
mp_face_detection = mp.solutions.face_detection
mp_drawing = mp.solutions.drawing_utils

# Initialize Face Detection model
face_detection = mp_face_detection.FaceDetection(model_selection=0, min_detection_confidence=0.5)

# Open the webcam
video = cv2.VideoCapture(0)

while True:
    success, frame = video.read()
    if not success:
        print("Ignoring empty camera frame.")
        continue

    # Convert the frame from BGR to RGB (MediaPipe uses RGB)
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # Process the frame to detect faces
    results = face_detection.process(rgb_frame)

    # Initialize face count
    face_count = 0

    # Check if any faces are detected
    if results.detections:
        face_count = len(results.detections)  # Count the number of faces

        # Draw bounding boxes around detected faces
        for detection in results.detections:
            mp_drawing.draw_detection(frame, detection)

    # Display the face count on the frame
    cv2.putText(frame, f'Faces: {face_count}', (50, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    # Show the frame with detections
    cv2.imshow("Face Detection", frame)

    # Break the loop if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the video capture and close windows
video.release()
cv2.destroyAllWindows()
