#!./virt/bin/python3
# -*- encoding: utf-8 -*-

import os
import uuid

from app import create_app, create_serve_app
from app.sockets import create_socket_app

import sys


class Config:
    DEBUG = False
    LOCAL = False
    basedir = os.path.abspath(os.path.dirname(__file__))
    passw = "password"
    auth = False
    port = 5001
    DEFAULT_DATA_PREFIX = "../build/"


app_config = Config()
app = create_app(app_config)
socketio = create_socket_app(app)
socketio.run(app, use_reloader=False, host="0.0.0.0", port=app_config.port)
