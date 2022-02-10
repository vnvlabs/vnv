#!./virt/bin/python3
# -*- encoding: utf-8 -*-

import os
from app import create_app
from app.sockets import create_socket_app
import sys

class Config:
    DEBUG = True
    LOCAL = False
    basedir = os.path.abspath(os.path.dirname(__file__))
    EXPLAIN_TEMPLATE_LOADING = True

app_config = Config()
app = create_app(app_config)

socketio = create_socket_app(app)

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 5001
    socketio.run(app,use_reloader=False, host="0.0.0.0", port=port)
