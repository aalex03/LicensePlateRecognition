import serial

# Serial port settings
port = "/dev/ttyUSB0"  # Use "/dev/ttyUSB0" for USB connection
baud_rate = 9600

# Open serial port
ser = serial.Serial(port, baud_rate)

try:
    while True:
        # Read a line from standard input
        data_to_send = input("Enter a line to send to Arduino: ")

        # Send data to Arduino
        ser.write(data_to_send.encode() + b'\n')

        # Read response from Arduino
        response = ser.readline().decode().strip()

        # Print received data
        print("Received from Arduino:", response)

except KeyboardInterrupt:
    # Close the serial port when interrupted
    ser.close()
