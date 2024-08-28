import serial

# Configure the serial port
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)  # Adjust port and baud rate as needed

try:
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip()  # Read a line and decode
            print(data)
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()
