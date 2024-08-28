
def split_and_sum_hex_values(hex_values):
    byte_sum = 0
    all_bytes = []

    for hex_value in hex_values:
        # Convert the hex string to an integer
        int_value = int(hex_value, 16)
        # Split the integer into individual bytes (8 bits per byte)
        for i in range(0, 8, 2): # 8 hex characters = 4 bytes
            byte = (int_value >> (i * 4)) & 0xFF
            all_bytes.append(byte)
            byte_sum += byte

    return all_bytes, byte_sum

hex_values = ["bcb39011" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3eb0cf" "bcc783a1" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3eb0cf" "bcc783a1" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3eb0cf" "bcb39011" "3d2f5ca3" "bda99ce8" "3d82c227" "3d796e46" "3d3eb0cf" "bcb39011" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3eb0cf" "bcb39011" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3eb0cf" "bcb39011" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3eb0cf" "bcc783a1" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3b9d71" "bcc783a1" "3d2c490c" "bda99ce8" "3d82c227" "3d796e46" "3d3eb0cf" "bcb39011" "3d2c490c" "bda99ce8" "3d844bf8" "3d796e46" "3d3eb0cf"]
bytes_list, total_sum = split_and_sum_hex_values(hex_values)

cropped_sum = total_sum & 0xFF

#actual_checkSum = 0x6E
#print(f"actual checksum: {hex(actual_checkSum)}")


print(f"Total sum: {hex(total_sum)}")
print(f"Cropped sum: {hex(cropped_sum)}")




#doubleCheck = 0x556E
#doublecheck_sum = doubleCheck & 0xFF
#print(f"double check sum: {doublecheck_sum}")

#float_hex_values = [0xC0F46A01, 0xC0337DEF, 0xC0C96249, 0xBDC65CA2, 0xBD8F19A6, 0x3E69DAAF]
#float_total_sum = sum(float_hex_values)
#print(f"Float total sum: {float_total_sum}")

