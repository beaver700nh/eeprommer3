import argparse
import os
import os.path
import serial
import time

import comm

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-p", "--port", help="specify port connected to eeprommer3", required=True)

    args = ap.parse_args()

    ser = open_port(args.port)
    com = comm.Comm(ser)

    try:
        main_loop(com)
    except KeyboardInterrupt:
        print("Thank you for using eeprommer3.")

def open_port(port: str):
    return serial.Serial(
        port=port,
        baudrate=115200,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=None
    )

def main_loop(com: comm.Comm):
    print("Ready.")

    active = None

    while True:
        pkt = com.recv()
        cmds = commands[active]

        try:
            active = cmds[pkt[0]](com, pkt[1:])
        except KeyError:
            print(f"Error: Action {pkt[0]} is invalid in current context.")

def cmd_none_ping(com: comm.Comm, args: bytes):
    print("Ping!")

    com.send(comm.PKT_PING)

    return None

current_file = {
    "name": "",
    "must_exist": False,
    "access": 0,
}

def cmd_none_fileopen(com: comm.Comm, args: bytes):
    prompt = com.recv().decode("ascii")
    name = file_dialog(prompt, args[0])

    print(f"Opening file `{name}', must_exist = {args[0]}")

    current_file["name"] = name
    current_file["must_exist"] = args[0]

    com.send(comm.PKT_FILEOPEN)

    return comm.PKT_FILEOPEN

def cmd_fileopen_fileconf(com: comm.Comm, args: bytes):
    print(f"File is accessed with flags 0x{args[0]:02x}")

    current_file["access"] = args[0]

    # TODO open the file

    return comm.PKT_FILECONF

def cmd_fileconf_filesize(com: comm.Comm, args: bytes):
    size = 0xABCD

    print("Querying file size: {size}")

    com.send(comm.PKT_FILESIZE, bytes([size & 0xFF, size >> 8]))

    return comm.PKT_FILECONF

def cmd_fileconf_fileseek(com: comm.Comm, args: bytes):
    position = pkt[1] | (pkt[2] << 8)

    print(f"Seeking to position 0x{position:04x}")

    return comm.PKT_FILECONF

def cmd_fileconf_fileread(com: comm.Comm, args: bytes):
    byte = 0xA5

    print(f"Reading byte from file: 0x{byte:02x}")

    com.send(comm.PKT_FILEREAD, bytes([byte]))

    return comm.PKT_FILECONF

def cmd_fileconf_filewrit(com: comm.Comm, args: bytes):
    print(f"Writing byte to file: 0x{args[0]:02x}")

    return comm.PKT_FILECONF

def cmd_fileconf_fileflus(com: comm.Comm, args: bytes):
    print("Flushing file")

    return comm.PKT_FILECONF

def cmd_fileconf_fileclos(com: comm.Comm, args: bytes):
    print("Closing file")

    return None

def file_dialog(prompt: str, must_exist: bool):
    while True:
        h, w = os.popen("stty size").read().split()
        path = os.popen(f"dialog --stdout --title {prompt!r} --fselect ~/ {h} {w}").read()

        if not must_exist or os.path.exists(path):
            return path

commands = {
    None:              {
        comm.PKT_PING:     cmd_none_ping,
        comm.PKT_FILEOPEN: cmd_none_fileopen,
    },
    comm.PKT_FILEOPEN: {
        comm.PKT_FILECONF: cmd_fileopen_fileconf,
    },
    comm.PKT_FILECONF: {
        comm.PKT_FILESIZE: cmd_fileconf_filesize,
        comm.PKT_FILESEEK: cmd_fileconf_fileseek,
        comm.PKT_FILEREAD: cmd_fileconf_fileread,
        comm.PKT_FILEWRIT: cmd_fileconf_filewrit,
        comm.PKT_FILEFLUS: cmd_fileconf_fileflus,
        comm.PKT_FILECLOS: cmd_fileconf_fileclos,
    },
}

if __name__ == "__main__":
    main()
