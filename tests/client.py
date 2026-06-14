import socket
import struct

# Connect to Mantis
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 8080))

# Mantis uses 8 decimal places for price precision
price_ticks = 100_000_000

# Construct a fake binary packet (Header + OrderPlacement)

# Magic(1), Version(1), Type(2), Len(2), Checksum(2)
header = struct.pack(">BBHHH", 0xCC, 1, 1, 32, 0)
# Account(8), Price(8), Qty(4), Side(1), Type(1), Symbol(8), Reserved(2)
payload = struct.pack(
    ">QQIBB8sH", 12345, int(143.72 * price_ticks), 50, 1, 1, b"AAPL", 0
)

s.sendall(header + payload)
s.close()
