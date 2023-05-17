import serial

PKT_PING     = 0x00
PKT_FILEOPEN = 0x10
PKT_FILECONF = 0x11
PKT_FILESIZE = 0x12
PKT_FILESEEK = 0x13
PKT_FILEREAD = 0x14
PKT_FILEWRIT = 0x15
PKT_FILEFLUS = 0x16
PKT_FILECLOS = 0x17

class Comm:
    def __init__(self, ser: serial.Serial):
        self.ser = ser

    def recv(self):
        end = self.ser.read()
        return self.ser.read(ord(end) + 1)

    def send_raw(self, data: bytes):
        self.ser.write(bytes([len(data) - 1]))
        self.ser.write(data)

    def send(self, header: int, buffer: bytes | None = None):
        if buffer is None:
            buffer = b""

        self.send_raw(bytes([header]) + buffer)
