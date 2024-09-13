import csv
import socket
import time
import os

# Directory for storing the CSV files
log_directory = 'raw'

# Ensure the log directory exists
os.makedirs(log_directory, exist_ok=True)

# Log data to a file with timestamped filenames
def log_data_to_csv(data, csv_file):

    # Strip newline characters and any extra spaces
    data = data.strip()

    # Split the data using commas and ensure there are exactly 3 values
    values = data.split(",")

    if len(values) == 3:
        try:
            xAcc, yAcc, zAcc = [float(value) for value in values]

            # Check if the file exists. If not, create and write headers
            file_exists = os.path.isfile(csv_file)

            # Write the data to the log file
            with open(csv_file, mode='a', newline='') as file:
                writer = csv.writer(file)

                # Write headers if the file is new
                if not file_exists:
                    writer.writerow(['xAcc', 'yAcc', 'zAcc']) # CSV column headers

                # Write the sensor data to the CSV file
                writer.writerow([xAcc, yAcc, zAcc])

            print(f"Data logged to {csv_file}: {data}\n")
        except ValueError as e:
            print(f"Error converting data to float: {e}, Data: {data}")
    else:
        print(f"Unexpected data format: {data}")

# Define server IP address and password
IP = '192.168.1.15'
PW = 'Sensor-Stream2024!'

# Create a TCP/IP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((IP, 8080)) # Bind to all interfaces and port 8080
server_socket.listen(1)
print("Waiting for connection...")

# Log file settings
log_file_name = f"sensor_data_{time.strftime('%Y%m%d-%H%M%S')}.csv"
log_file = os.path.join(log_directory, log_file_name) 


# Function to rotate log files every 10 seconds
def rotate_log_file():
    global log_file
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    log_file = f"sensor_data_{timestamp}.log" # New log file for the next session

# Wait for connection
connection, client_address = server_socket.accept()
print(f"Connection from {client_address}")

try:
    # Receive and check the password
    received_password = connection.recv(1024).decode()
    if received_password != PW:
        print("Unauthorized access attempt.")
        connection.close()
    else:
        while True:
            data = connection.recv(1024)
            if not data:
                break
            print(f"Received: {data.decode()}")

            # Log the data to the file
            log_data_to_csv(data.decode(), log_file)

finally:
    connection.close()
    server_socket.close()