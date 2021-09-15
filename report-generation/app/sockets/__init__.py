
from flask import Flask, render_template, request
from flask_socketio import SocketIO


def create_socket_app(app):

    __app__ = app
    socketio = SocketIO(app)
    from .namepsaces.term import configure_terminal
    from .namepsaces.ipython import configure_terminal as pyconf

    configure_terminal(socketio, app)
    pyconf(socketio, app)

    @app.route('/terminal')
    def terminal():
        pt = request.args.get("term", "")
        return render_template("includes/term.html", pty=pt + "pty")

    return socketio
