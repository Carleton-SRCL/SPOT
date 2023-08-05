import socket
import struct
import time

IP_ADDRESS = '192.168.1.112' # Set the IP address to receive data from (in this case, BLUE)
PORT = 46875
NUM_DOUBLES = 19

# Create a UDP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the port
server_address = (IP_ADDRESS, PORT)
print('Starting up on {} port {}'.format(server_address[0], server_address[1]))
server_socket.bind(server_address)

# Set the socket to non-blocking
server_socket.setblocking(0)

data = b''

try:
    while True:
        try:
            # Receive the data
            more_data, client_address = server_socket.recvfrom(NUM_DOUBLES * 8 - len(data))
            if more_data:
                data += more_data
            else:
                break

            # Unpack and display the data if enough bytes have been received
            if len(data) == NUM_DOUBLES * 8:
                doubles = struct.unpack('d' * NUM_DOUBLES, data)
                print('Received doubles: {}'.format(doubles))
                data = b''  # Clear the data buffer for the next set of doubles

        except BlockingIOError:
            # No data received, continue waiting
            time.sleep(0.1)  # Optional: add a small delay to reduce CPU usage

except KeyboardInterrupt:
    print('\nExiting...')

# Close the socket
server_socket.close()
