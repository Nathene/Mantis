import socket
import struct

# 1. Pack the MessageHeader
# Format: < (little-endian), B (uint8), B (uint8), H (uint16), H (uint16), H (uint16)
# magic_byte (0xCC), version (1), message_type (1), message_length (32), checksum (0)
header = struct.pack("<BBHHH", 0xCC, 1, 1, 32, 0)

# 2. Pack the OrderPlacement Payload
# Format: 8s (8 chars), Q (uint64), d (double), I (uint32), B (uint8), B (uint8), H (uint16)
# Notice the exact 8-byte string for the ticker: b'AAPL\0\0\0\0'
payload = struct.pack("<8sQdIBBH", b"AAPL\0\0\0\0", 999123, 150.25, 500, 1, 1, 0)

# 3. Connect and send the batch over TCP
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", 8080))
    s.sendall(header + payload)
    print("Payload fired successfully!")
    s.close()
except ConnectionRefusedError:
    print("Connection failed. Is the Mantis server running?")
