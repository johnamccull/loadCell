import can
import csv
import time
import subprocess
import struct

# Configuration
can_interface = 'can0'
output_file = 'can_data.csv'
duration = 5  # Duration in seconds to log CAN data

can_id = 0x080  # The CAN ID
stop_command = [0x00] # stop data stream
sing_command  =  [0x01]  # single data stream
continuous_command = [0x02]   # start data stream

# Function to run a command
def run_command(command):
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        exit(1)

# Initialize CAN interface
commands = [
    'sudo modprobe can',
    'sudo modprobe can-dev',
    'sudo modprobe can-raw',
    'sudo ip link set can0 up type can bitrate 1000000', #setting bitrate of local machine (1Mb/s is what the load cell uses)
    'sudo ifconfig can0 up'
]

print("Initializing CAN interface...")
for cmd in commands:
    run_command(cmd)

# Set up CAN bus
try:
    bus = can.interface.Bus(can_interface, interface='socketcan')
    
    bus.send(can.Message(arbitration_id=can_id, data=continuous_command, is_extended_id=False))
    # in terminal this would be cansend can0 080#02

    # Open a CSV file to save the data
    with open(output_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Write header
        writer.writerow(['Timestamp', 'Fx', 'Fy', 'Fz', 'Mx', 'My', 'Mz'])

        print(f"Listening on {can_interface}... Press Ctrl+C to stop.")

        start_time = time.time()
        messages_buffer = []

        try:
            while time.time() - start_time < duration:
                # Wait for a CAN message
                message = bus.recv(timeout=1)

                if message:
                    # Extract data
                    timestamp = message.timestamp
                    can_id = message.arbitration_id
                    data = message.data.hex()

                    message_bytes = bytes.fromhex(data)
                    float1 = struct.unpack('<f', message_bytes[:4])[0]
                    float2 = struct.unpack('<f', message_bytes[4:])[0]

                    messages_buffer.append((float1, float2))

                    if len(messages_buffer) == 3:
                        # Flatten the buffer and write the row
                        row = [item for sublist in messages_buffer for item in sublist]
                        writer.writerow([timestamp] + row)

                        # Print the received message (optional)
                        # print(row)
    
                        # Clear the buffer
                        messages_buffer = []


        except KeyboardInterrupt:
            print("\nInterrupted by user. Exiting...")
        finally:
            bus.send(can.Message(arbitration_id=can_id, data=stop_command, is_extended_id=False))
            print(f"Data saved to {output_file}")
except can.CanError as e:
    print(f"Failed to initialize CAN bus: {e}")
finally:
    # Properly shutdown the CAN bus
    if 'bus' in locals():
        bus.shutdown()
        print("CAN bus properly shut down.")