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
    while True:
        pkt = com.recv()

        if pkt[:1] == comm.PKT_PING:
            com.send(comm.PKT_PING);

if __name__ == "__main__":
    main()
