import socket

# Define server IP address and password
IP = '192.168.1.15'
PW = 'Sensor-Stream2024!'

# Create a TCP/IP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((IP, 8080)) # Bind to all interfaces and port 8080
server_socket.listen(1)
print("Waiting for connection...")

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
finally:
    connection.close()
    server_socket.close()