import fcntl
import os
import pty
import select
import struct
import subprocess
import termios
from shlex import shlex

from IPython.terminal.embed import InteractiveShellEmbed

from app.sockets.namepsaces.general import read_and_forward_pty_output, set_winsize


def configure_terminal(socketio, app):

    def read_and_forward():
        read_and_forward_pty_output(socketio, app, "pty", "fd")

    @socketio.on("pty-input", namespace="/pty")
    def pty_input(data):
        if app.config["fd"]:
            os.write(app.config["fd"], data["input"].encode())

    @socketio.on("resize", namespace="/pty")
    def resize(data):
        if app.config["fd"]:
            set_winsize(app.config["fd"], data["rows"], data["cols"])

    @socketio.on("connect", namespace="/pty")
    def connect():
        if "child_pid" in app.config and app.config["child_pid"]:
            return

        (child_pid, fd) = pty.fork()
        if child_pid == 0:
            subprocess.run("bash", cwd=os.getenv("SOURCE_DIR"))
        else:
            # this is the parent process fork.
            # store child fd and pid
            app.config["fd"] = fd
            app.config["child_pid"] = child_pid
            set_winsize(fd, 50, 50)
            socketio.start_background_task(target=read_and_forward)
