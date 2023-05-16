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
    print("-> Pong!")

    return None

def cmd_none_fileopen(com: comm.Comm, args: bytes):
    prompt = com.recv().decode("ascii")
    input(f"Prompt: {prompt} (press enter)")

    path = file_dialog(prompt, args[0])
    current_file.path = path

    print(f"-> Opening file `{path}', must_exist = {args[0]}")

    com.send(comm.PKT_FILEOPEN)

    return comm.PKT_FILEOPEN

def cmd_fileopen_fileconf(com: comm.Comm, args: bytes):
    print(f"-> File is configured with flags 0x{args[0] :02X}")

    current_file.descriptor = os.open(current_file.path, args[0])

    return comm.PKT_FILECONF

def cmd_fileconf_filesize(com: comm.Comm, args: bytes):
    print("Querying file size")

    size = os.fstat(current_file.descriptor).st_size
    print(f"-> Size is 0x{size :04X} bytes")

    com.send(comm.PKT_FILESIZE, bytes([size & 0xFF, size >> 8]))

    return comm.PKT_FILECONF

def cmd_fileconf_fileseek(com: comm.Comm, args: bytes):
    position = args[1] | (args[2] << 8)

    print(f"Seeking to position 0x{position :04X}")
    os.lseek(current_file.descriptor, position, os.SEEK_START)

    return comm.PKT_FILECONF

def cmd_fileconf_fileread(com: comm.Comm, args: bytes):
    print(f"Reading 0x{args[0] + 1 :04X} bytes from file")

    data = os.read(current_file.descriptor, args[0] + 1)
    print(f"-> Data is {hexdump(data, '-- ........ ')}")

    com.send(comm.PKT_FILEREAD, data)

    return comm.PKT_FILECONF

def cmd_fileconf_filewrit(com: comm.Comm, args: bytes):
    print(f"Writing bytes to file")

    pkt = com.recv()
    print(f"-> Count 0x{len(pkt) :04X} bytes")
    print(f"-> Data is {hexdump(pkt, '-- ....... ')}")

    os.write(current_file.descriptor, pkt)

    return comm.PKT_FILECONF

def cmd_fileconf_fileflus(com: comm.Comm, args: bytes):
    print("Flushing file")

    os.fsync(current_file.descriptor)

    return comm.PKT_FILECONF

def cmd_fileconf_fileclos(com: comm.Comm, args: bytes):
    print("Closing file")

    os.close(current_file.descriptor)

    return None

def file_dialog(prompt: str, must_exist: bool):
    while True:
        h, w = os.popen("stty size").read().split()
        path = os.popen(f"dialog --stdout --title {prompt!r} --fselect ~/ {h} {w}").read()

        if not must_exist or os.path.exists(path):
            return path

        input("The file must exist! (press enter)")

def hexdump(data: bytes, prefix: str):
    buffer = [
        (""            if i      == 0 else
        (f"\n{prefix}" if i % 16 == 0 else
        (" "           if i %  8 == 0 else
        ("")))) +
        f"{x :02X}"
        for i, x in enumerate(data)
    ]

    return " ".join(buffer)

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

class File:
    __slots__ = "path", "descriptor"

current_file = File()

if __name__ == "__main__":
    try:
        main()
    except serial.serialutil.SerialException as e:
        print(f"Error: {e}")
