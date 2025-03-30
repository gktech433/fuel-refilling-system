import cv2
import requests
import os
import boto3
from botocore.exceptions import ClientError
import uuid
from dotenv import load_dotenv
load_dotenv()
from azure.ai.vision.imageanalysis import ImageAnalysisClient
from azure.ai.vision.imageanalysis.models import VisualFeatures
from azure.core.credentials import AzureKeyCredential
import psycopg2

def capture_image(type="vehicle"):
    """Captures an image using the webcam and saves it."""
    cap = cv2.VideoCapture(0)
    image_name=uuid.uuid4()
    image_path = None
    if type == "vehicle":
        image_path = f"images/vehicle_reg_numbers/{image_name}.jpg"
    elif type == "meter":
        image_path = f"images/meter_readings/{image_name}.jpg"
    if not cap.isOpened():
        print("Error: Could not access the webcam.")
        return None
    
    ret, frame = cap.read()
    if ret:
        cv2.imwrite(image_path, frame)
        print(f"Image saved as {image_path}")
    else:
        print("Error: Could not capture an image.")
        image_path = None
    
    cap.release()
    return image_name

def get_vehicle_reg_number(image_path):
    try:
        endpoint = os.environ["VISION_ENDPOINT"]
        key = os.environ["VISION_KEY"]
    except KeyError:
        print("Missing environment variable 'VISION_ENDPOINT' or 'VISION_KEY'")
        exit()

    # Create an Image Analysis client
    client = ImageAnalysisClient(
        endpoint=endpoint,
        credential=AzureKeyCredential(key)
    )

    with open(image_path, "rb") as f:
        image_data = f.read()

    result = client.analyze(
        image_data=image_data,
        visual_features=[VisualFeatures.READ]
    )

    vehicle_reg_number=None
    if result.read is not None:
        for line in result.read.blocks[0].lines:
            print(f"Line: '{line.text}'")
            vehicle_reg_number=line.text
    else:
        print("OCR operation failed or timed out")
        
    return vehicle_reg_number

def get_db_connection():
    try:
        DB_HOST = os.environ["DB_HOST"]
        DB_NAME = os.environ["DB_NAME"]
        DB_PASSWORD = os.environ["DB_PASSWORD"]
        DB_PORT = os.environ["DB_PORT"]
        DB_USER = os.environ["DB_USER"]
        return psycopg2.connect(
            database=DB_NAME,
            user=DB_USER,
            password=DB_PASSWORD,
            host=DB_HOST,
            port=DB_PORT,
        )
    except:
        return False

def get_vehicle_status(vehicle_reg_number):
    conn = get_db_connection()
    if conn:
        print("DB connected.")
    else:
        print("Unable to connect DB")
    curr = conn.cursor()
    
    curr.execute("SELECT * FROM vehicles WHERE vehicle_no = %s", (vehicle_reg_number,))
    
    data = curr.fetchall()
    if len(data) > 0:
        return True
    return False

def get_initial_meter_reading():
    


def upload_image(image_name):
    # Upload the file
    session = boto3.Session(profile_name='vehicle')
    s3_client = session.client('s3')
    bucket="fuel-filling-system-images"
    object_key=f"number-plates/{image_name}.jpg"
    image_path=f"images/vehicle_reg_numbers/{image_name}.jpg"
    try:
        response = s3_client.upload_file(image_path, bucket, object_key)
    except ClientError as e:
        logging.error(e)
        return False
    return True

if __name__ == "__main__":
    image_name = capture_image()
    if image_name is None:
        print("no image name")
    image_path=f"images/vehicle_reg_numbers/{image_name}.jpg"
    vehicle_reg_number=get_vehicle_reg_number(image_path)
    # vehicle_reg_number=get_vehicle_reg_number("images/noplate5.jpg")
    print(vehicle_reg_number)
    vehicle_status=get_vehicle_status(vehicle_reg_number)
    print(vehicle_status)
