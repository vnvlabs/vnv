#!./virt/bin/python3
# -*- encoding: utf-8 -*-

import os
import uuid

from app import create_app, create_serve_app
from app.sockets import create_socket_app

import sys


class Config:
    DEBUG = True
    LOCAL = False
    basedir = os.path.abspath(os.path.dirname(__file__))
    passw = sys.argv[2] if len(sys.argv) > 2 else "password"
    auth = False
    port = 5001
    DEFAULT_DATA_PREFIX = "../build/"


app_config = Config()

if len(sys.argv) > 1 and sys.argv[1] == "serve":

    app_config.DEBUG = False
    socketio, app = create_serve_app(app_config)

else:

    if len(sys.argv) > 1 and sys.argv[1] == "container":
        app_config.DEBUG = False
        app_config.DEFAULT_DATA_PREFIX = "/home/vnv/"


    app = create_app(app_config)
    socketio = create_socket_app(app)

socketio.run(app, use_reloader=False, host="0.0.0.0", port=app_config.port)
