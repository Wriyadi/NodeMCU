import cv2
import mediapipe as mp
import numpy as np

# Initialize MediaPipe Object Detection components
mp_vision = mp.tasks.vision
mp_image = mp.Image
mp_object_detector = mp_vision.ObjectDetector

# Load the TFLite model (e.g., efficientdet_lite0.tflite)
base_options = mp.tasks.BaseOptions(model_asset_path='efficientdet_lite0.tflite')
detector_options = mp_vision.ObjectDetectorOptions(
    base_options=base_options,
    max_results=10,  # Maximum number of detected objects
    score_threshold=0.5  # Confidence threshold
)

# Create an Object Detector instance
with mp_object_detector.create_from_options(detector_options) as detector:
    # Open the webcam
    video = cv2.VideoCapture(0)

    while True:
        success, frame = video.read()
        if not success:
            print("Ignoring empty camera frame.")
            continue

        # Convert the frame to RGB (as MediaPipe expects RGB input)
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # Convert RGB frame to MediaPipe Image format
        mp_frame = mp_image(image_format=mp.ImageFormat.SRGB, data=rgb_frame)

        # Perform object detection
        detection_result = detector.detect(mp_frame)

        # Get the number of objects detected
        object_count = len(detection_result.detections)

        # Draw bounding boxes and labels for detected objects
        for detection in detection_result.detections:
            bbox = detection.bounding_box
            x1, y1 = int(bbox.origin_x), int(bbox.origin_y)
            x2, y2 = int(bbox.width + x1), int(bbox.height + y1)

            # Draw bounding box on the frame
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

            # Extract label and confidence score
            category = detection.categories[0]
            label = category.category_name
            score = category.score

            # Display label and score
            cv2.putText(frame, f'{label} ({score:.2f})', (x1, y1 - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        # Display the object count on the frame
        cv2.putText(frame, f'Total Objects: {object_count}', (50, 50),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # Show the frame with detections
        cv2.imshow("Object Detection", frame)

        # Break the loop if 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release the video capture and close windows
    video.release()
    cv2.destroyAllWindows()