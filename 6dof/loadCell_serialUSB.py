import csv
import serial
import time  
import struct 
import matplotlib.pyplot as plt

cmd_samp_rate_200 = [0x41, 0x54, 0x2B, 0x53, 0x4D, 0x50, 0x52, 0x3D, 0x32, 0x30, 0x30, 0x0D, 0x0A]
cmd_samp_rate_300 = [0x41, 0x54, 0x2B, 0x53, 0x4D, 0x50, 0x52, 0x3D, 0x33, 0x30, 0x30, 0x0D, 0x0A] # "AT+SMPR=300\r\n"
cmd_start_streaming = [
    0x41, 0x54, 0x2B, 0x47, 0x45, 0x54, 0x53, 0x44, 0x3D, 0x28, 0x41,
    0x30, 0x31, 0x2C, 0x41, 0x30, 0x32, 0x2C, 0x41, 0x30, 0x33, 0x2C, 0x41, 0x30, 0x34, 0x2C,
    0x41, 0x30, 0x35, 0x2C, 0x41, 0x30, 0x36, 0x29, 0x3B, 0x4D, 0x3B, 0x31, 0x30, 0x0D, 0x0A] # "AT+GETSD=(A01,A02,A03,A04,A05,A06);M;10\r\n"
cmd_stop_streaming = [0x41, 0x54, 0x2B, 0x47, 0x45, 0x54, 0x53, 0x44, 0x3D, 0x53, 0x54, 0x4F, 0x50, 0x0D, 0x0A] # "AT+GETSD=STOP\r\n"

def find_packet_start(serial_port):
    """Align to the packet start, looking for the 0xAA 0x55 header."""
    while True:
        byte1 = serial_port.read(1)
        if byte1 == b'\xAA':
            byte2 = serial_port.read(1)
            if byte2 == b'\x55':
                return
            
def read_packet(serial_port):
    """Read a single packet from the serial port."""
    # Align to the start of a packet
    find_packet_start(serial_port)
    
    # Read the length of the packet (next 2 bytes)
    length_bytes = serial_port.read(2)

    packet_length = struct.unpack('>H', length_bytes)[0]  # big-endian, unsigned short
    # Print the length of the packet
    #print(f"Packet Length: {packet_length} (Hex: {length_bytes.hex().upper()})")
    #print(f"Packet Length: {packet_length}")

    # Read the packet number (next 2 bytes)
    packet_num_bytes = serial_port.read(2)
    packet_num = struct.unpack('>H', packet_num_bytes)[0]  # little-endian, unsigned short
    packet_num_binary = format(packet_num, 'b')
    # Print the length of the packet
    #print(f"Packet Number: {packet_num} (Hex: {packet_num_bytes.hex().upper()})")
    #print(f"Packet Number: {packet_num}")

    # Read the remaining data based on the packet length
    data_bytes = serial_port.read(packet_length-3) #1 for checksum, 2 for packet number

    return data_bytes

def segment_into_floats(data_bytes):
    """Segment the data into single-precision floats and distribute them into 6 channel buffers."""
    #print(f"Original data_bytes (Hex): {data_bytes.hex().upper()}")
    
    num_floats = len(data_bytes) // 4  # 4 bytes per float
    
    # Flip the byte order for each 4-byte float
    flipped_data = bytearray()
    for i in range(num_floats):
        float_bytes = data_bytes[i*4:(i+1)*4]
        flipped_data.extend(float_bytes[::-1])  # Reverse the order of the bytes

    # Convert the flipped byte data to floats
    floats = struct.unpack('>' + 'f' * num_floats, flipped_data) # little-endian floats

    # Create buffers for 6 channels
    num_channels = 6
    channel_buffers = [[] for _ in range(num_channels)]

    # Distribute the floats into 6 different buffers
    for i, value in enumerate(floats):
        channel_index = i % num_channels
        channel_buffers[channel_index].append(value) #range between 0-2000 now

    # Print each channel's buffer at the end
    for i, buffer in enumerate(channel_buffers):
        if buffer:  # Check if the buffer is not empty
            scaled_val = int(abs(buffer[-1]*100))
            #print(f"Channel {i+1} ", end="")
            #for i in range(scaled_val):
            #    print("(^_^)", end="")  # All prints will be on the same line
            #print()
            #print(f"Channel {i+1} Last Value: {buffer[-1]}")
    return channel_buffers

def read_from_m8123b2(port, baudrate, parity, stopbits, bytesize):
    ser = serial.Serial(port=port, baudrate=baudrate, parity=parity, stopbits=stopbits, bytesize=bytesize, timeout=1)
    print(f"Serial port {port} opened successfully.")


    ser.write(bytearray(cmd_samp_rate_300))
    print(f"Sent data: {cmd_samp_rate_300}")
    
    time.sleep(1)
    if ser.in_waiting > 0:
        response = ser.readline().decode('utf-8').rstrip()
        print(f"Response after sending data1: {response}")
    
    ser.write(bytearray(cmd_start_streaming))
    print(f"Sent data: {cmd_start_streaming}")
    
    all_channel_data = [[] for _ in range(6)]  # To store all data for each channel

    try:
        while True:
            packet_data = read_packet(ser)
            floats = segment_into_floats(packet_data)
            
            # Store the data in all_channel_data
            for i in range(6):
                all_channel_data[i].extend(floats[i])
            
    except serial.SerialException as e:
        print(f"Error reading from M8123B2: {e}")
    except KeyboardInterrupt:
        print("\nKeyboard interruption detected. Exiting the program.")
    finally:
        ser.write(bytearray(cmd_stop_streaming))
        print(f"Sent data: {cmd_stop_streaming}")
        if ser.is_open:
            ser.close()
            print(f"Serial port {port} closed.")

        # Plot the data at the end
        fig, axs = plt.subplots(6, 1, figsize=(10, 12))
        for i in range(6):
            axs[i].plot(all_channel_data[i])
            axs[i].set_title(f'Channel {i+1}')
            axs[i].set_xlabel('Sample Number')
            axs[i].set_ylabel('Amplitude')
        plt.tight_layout()
        plt.show()


if __name__ == "__main__":
    port = '/dev/ttyUSB0'  # Adjust for your system
    baudrate = 115200
    parity = serial.PARITY_NONE
    stopbits = serial.STOPBITS_ONE
    bytesize = serial.EIGHTBITS

    read_from_m8123b2(port, baudrate, parity, stopbits, bytesize)
