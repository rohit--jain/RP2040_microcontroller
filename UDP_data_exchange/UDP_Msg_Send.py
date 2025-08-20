import socket

# Define the UDP IP and port
UDP_IP = "192.168.178.57"  # Localhost
UDP_PORT = 5005       # Port number

# Message to be sent
MESSAGE = b"Hello, World!"  # Note: Message must be in bytes

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    # Send the message
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    print(f"Sent message: {MESSAGE} to {UDP_IP}:{UDP_PORT}")
