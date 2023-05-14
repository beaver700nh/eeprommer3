import serial

PKT_PING = b"\x00"

class Comm:
    def __init__(self, ser: serial.Serial):
        self.ser = ser

    def recv(self):
        end = self.ser.read()
        return self.ser.read(ord(end) + 1)

    def send(self, buffer: bytes):
        self.ser.write(len(buffer) - 1)
        self.ser.write(buffer)
