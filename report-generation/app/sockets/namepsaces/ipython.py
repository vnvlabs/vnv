import fcntl
import os
import pty
import select
import struct
import termios
from shlex import shlex

from IPython.terminal.embed import InteractiveShellEmbed

from app.sockets.namepsaces.general import read_and_forward_pty_output, set_winsize


def configure_terminal(socketio, app):

    def read_and_forward():
        read_and_forward_pty_output(socketio, app, "pypty", "pyfd")

    @socketio.on("pypty-input", namespace="/pypty")
    def pypty_input(data):
        if app.config["pyfd"]:
            os.write(app.config["pyfd"], data["input"].encode())

    @socketio.on("resize", namespace="/pypty")
    def resize(data):
        if app.config["pyfd"]:
            set_winsize(app.config["pyfd"], data["rows"], data["cols"])

    @socketio.on("connect", namespace="/pypty")
    def connect():

        if "py_child_pid" in app.config and app.config["py_child_pid"]:
            # already started child process, don't start another
            return

        (child_pid, fd) = pty.fork()
        if child_pid == 0:
            ipshell = InteractiveShellEmbed(banner1="", exit_msg="bye")
            ipshell()
        else:
            # this is the parent process fork.
            # store child fd and pid
            app.config["pyfd"] = fd
            app.config["py_child_pid"] = child_pid
            set_winsize(fd, 50, 50)
            socketio.start_background_task(target=read_and_forward)
