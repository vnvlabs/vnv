import fcntl
import os
import select
import struct
import termios


def set_winsize(fd, row, col, xpix=0, ypix=0):
    winsize = struct.pack("HHHH", row, col, xpix, ypix)
    fcntl.ioctl(fd, termios.TIOCSWINSZ, winsize)


def read_and_forward_pty_output(socketio, app, pty, fd, ):
    max_read_bytes = 1024 * 20
    while True:
        socketio.sleep(0.01)
        if app.config[fd]:
            timeout_sec = 0
            (data_ready, _, _) = select.select(
                [app.config[fd]], [], [], timeout_sec)
            if data_ready:
                output = os.read(app.config[fd], max_read_bytes).decode()
                socketio.emit(f"{pty}-output",
                              {"output": output},
                              namespace=f"/{pty}")
