import csv
import socket
import time
import os

# Directory for storing the CSV files
log_directory = 'raw'

# Ensure the log directory exists
os.makedirs(log_directory, exist_ok=True)

# Create different log files for each sensor
timestamp = time.strftime('%Y%m%d-%H%M%S')
acc_log_file = os.path.join(log_directory, f"acc_{timestamp}.csv")
gyro_log_file = os.path.join(log_directory, f"gyro_{timestamp}.csv")
mag_log_file = os.path.join(log_directory, f"mag_{timestamp}.csv")

# Log data to a CSV file based on sensor type
def log_data_to_csv(data):

    # Split the data by newline characters to handle multiple lines
    lines = data.strip().split("\n")

    for line in lines:
        # Strip any extra spaces and split the data using commas 
        values = line.strip().split(",")

        # Ensure the data format and sensor type matches
        if len(values) == 4 and values[0] in ["Acc", "Gyro", "Mag"]:
            try:
                sensor_type, x, y, z = values
                x, y, z = float(x), float(y), float(z)

                # Determine the right log file
                if sensor_type == "Acc":
                    csv_file = acc_log_file
                elif sensor_type == "Gyro":
                    csv_file = gyro_log_file
                else: # if sensor_type == "Mag"
                    csv_file = mag_log_file

                # Check if the file exists. If not, create and write headers
                file_exists = os.path.isfile(csv_file)

                # Write the data to the log file
                with open(csv_file, mode='a', newline='') as file:
                    writer = csv.writer(file)

                    # Write headers if the file is new
                    if not file_exists:
                        writer.writerow([f'x{sensor_type}', f'y{sensor_type}', f'z{sensor_type}']) # CSV column headers

                    # Write the sensor data to the CSV file
                    writer.writerow([x, y, z])

                print(f"Data logged to {csv_file}: {line}\n")
            except ValueError as e:
                print(f"Error converting data to float: {e}, Data: {line}")
        else:
            print(f"Unexpected data format or unrecognized sensor_type: {line}")

# Define server IP address and session password
IP = "192.168.1.15"
PW = input("Please create a password for the session: ")

def start_server():
    # Create a TCP/IP socket 
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((IP, 8080)) # Bind to all interfaces and port 8080
    server_socket.listen(1)
    print("Waiting for connection...")

    try:
        while True:
            # Wait for connection
            connection, client_address = server_socket.accept()
            print(f"Connection established with {client_address}")

            try:
                # Receive and check the password
                received_password = connection.recv(1024).decode()
                print(f"Received password: '{received_password}'")  # Debugging statement

                if received_password != PW:
                    print("Unauthorized access attempt.")
                    connection.sendall(b"UNAUTHORIZED")
                    connection.close()  # Close only the client connection
                else:
                    print("Authorized access. Ready to receive data.")
                    connection.sendall(b"AUTHORIZED")
                    while True:
                        data = connection.recv(1024)
                        if not data:
                            break
                        print(f"Received data: {data.decode()}")

                        # Log data
                        log_data_to_csv(data.decode())

            finally:
                connection.close()  # Always close the client connection

    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Close the server socket only when you want to stop the server
        server_socket.close()

if __name__ == "__main__":
    start_server()