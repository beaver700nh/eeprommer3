import argparse
import os.path
import serial
import time

import comm

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-p", "--port", help="specify port connected to eeprommer3", required=True)

    args = ap.parse_args()

    wait_port(args.port)

    ser = serial.Serial(
        port=args.port,
        baudrate=115200,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=None
    )

    com = comm.Comm(ser)

    main_loop(com)

def wait_port(port: str):
    print(f"Waiting for connection on `{port}'...");

    while not os.path.exists(port):
        pass

    print("Ready.")

def main_loop(com: comm.Comm):
    active = None
    filename = None
    fileexist = None
    fileaccess = None
    file = None

    while True:
        pkt = com.recv()

        if active is None:
            if pkt[0] == comm.PKT_PING:
                com.send(comm.PKT_PING)
            elif pkt[0] == comm.PKT_FILEOPEN:
                active = comm.PKT_FILEOPEN
                input("Press enter.")
                fileexist = pkt[1]
                com.send(comm.PKT_FILEOPEN)
        elif active == comm.PKT_FILEOPEN:
            print("Open", pkt.decode("ascii"))

            pkt = com.recv()

            if pkt[0] == comm.PKT_FILECONF:
                active = comm.PKT_FILECONF
                fileaccess = pkt[1]
        elif active == comm.PKT_FILECONF:
            if pkt[0] == comm.PKT_FILESIZE:
                com.send(comm.PKT_FILESIZE, b"\xCD\xAB")
            elif pkt[0] == comm.PKT_FILESEEK:
                print("Seek", pkt[1] | (pkt[2] << 8))
            elif pkt[0] == comm.PKT_FILEREAD:
                com.send(comm.PKT_FILEREAD, b"\xA5")
            elif pkt[0] == comm.PKT_FILEWRIT:
                print("Write", pkt[1])
            elif pkt[0] == comm.PKT_FILEFLUS:
                print("Flush")
            elif pkt[0] == comm.PKT_FILECLOS:
                active = None
                print("Close")

if __name__ == "__main__":
    main()
