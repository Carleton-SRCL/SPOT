
import RPi.GPIO as GPIO
import socket
import struct
import time
import signal

# Define the 8 pins you want to use.
PINS = [7, 12, 13, 15, 16, 18, 22, 23]

# Set up the GPIO library
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)

for pin in PINS:
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.LOW)

# UDP setup
IP_ADDRESS = '127.0.0.1'
PORT = 48291
NUM_DOUBLES = 10

server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = (IP_ADDRESS, PORT)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(server_address)
server_socket.setblocking(0)

data = b''

def signal_handler(sig, frame):
    raise KeyboardInterrupt

signal.signal(signal.SIGTERM, signal_handler)

SAFETY_BIT = 568471
duty_cycles = [0] * len(PINS)
pwm_frequency = 50  # Default to 50Hz

# Initialize timestamps and states for each pin
next_toggle_time = [0] * len(PINS)
pin_states = [GPIO.LOW] * len(PINS)

try:
    while True:
        current_time = time.time()

        for i, pin in enumerate(PINS):
            if current_time >= next_toggle_time[i]:
                if pin_states[i] == GPIO.LOW:
                    pin_states[i] = GPIO.HIGH
                    next_toggle_time[i] = current_time + (duty_cycles[i] / 100) * (1 / pwm_frequency)
                else:
                    pin_states[i] = GPIO.LOW
                    next_toggle_time[i] = current_time + (1 - duty_cycles[i] / 100) * (1 / pwm_frequency)
                
                GPIO.output(pin, pin_states[i])

        # Check for new data
        try:
            more_data, client_address = server_socket.recvfrom(NUM_DOUBLES * 8 - len(data))
            if more_data:
                data += more_data
            if len(data) == NUM_DOUBLES * 8:
                doubles = struct.unpack('d' * NUM_DOUBLES, data)
                if int(doubles[0]) == SAFETY_BIT:
                    pwm_frequency = doubles[1]
                    duty_cycles = doubles[2:10]
                data = b''
        except BlockingIOError:
            pass

except KeyboardInterrupt:
    GPIO.output(PINS[0],GPIO.LOW)
    GPIO.output(PINS[1],GPIO.LOW)
    GPIO.output(PINS[2],GPIO.LOW)
    GPIO.output(PINS[3],GPIO.LOW)
    GPIO.output(PINS[4],GPIO.LOW)
    GPIO.output(PINS[5],GPIO.LOW)
    GPIO.output(PINS[6],GPIO.LOW)
    GPIO.output(PINS[7],GPIO.LOW)
    GPIO.cleanup()
    server_socket.close()
    print("\nExiting...")

finally:
    GPIO.output(PINS[0],GPIO.LOW)
    GPIO.output(PINS[1],GPIO.LOW)
    GPIO.output(PINS[2],GPIO.LOW)
    GPIO.output(PINS[3],GPIO.LOW)
    GPIO.output(PINS[4],GPIO.LOW)
    GPIO.output(PINS[5],GPIO.LOW)
    GPIO.output(PINS[6],GPIO.LOW)
    GPIO.output(PINS[7],GPIO.LOW)
    GPIO.cleanup()
    server_socket.close()

