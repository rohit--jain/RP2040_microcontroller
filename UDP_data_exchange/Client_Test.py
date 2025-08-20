import socket

PICO_IP = "192.168.1.50"    # Replace with your Pico W IP (check router or serial output)
PICO_PORT = 5005            # Must match the Pico code
PC_PORT = 5005              # Same as Pico's UDP_LOCAL_PORT

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", PC_PORT))  # Bind to all local interfaces

print(f"Listening for UDP messages on port {PC_PORT}...")
print(f"Sending messages to Pico at {PICO_IP}:{PICO_PORT}")

try:
    while True:
        # Send a test message to Pico
        message = "Hello from PC!"
        sock.sendto(message.encode(), (PICO_IP, PICO_PORT))
        print(f"Sent: {message}")

        # Receive a response (timeout 2s)
        sock.settimeout(2.0)
        try:
            data, addr = sock.recvfrom(1024)
            print(f"Received from {addr}: {data.decode()}")
        except socket.timeout:
            print("No response received.")

        print("---")
except KeyboardInterrupt:
    print("\nExiting...")
    sock.close()