#!./virt/bin/python3
# -*- encoding: utf-8 -*-

import os
from app import create_app
from app.sockets import create_socket_app


class Config:
    DEBUG = True
    LOCAL = False
    basedir = os.path.abspath(os.path.dirname(__file__))


app_config = Config()
app = create_app(app_config)

socketio = create_socket_app(app)

if __name__ == "__main__":
    socketio.run(app,use_reloader=False)
