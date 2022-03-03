# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from flask import Flask,render_template
from logging import DEBUG

from flask_socketio import SocketIO
from flaskext.markdown import Markdown

global_template_variables = {

}


def register_blueprints(apps, config):

    import app.base
    from .base.utils.mongo import Configured
    from .models.VnV import DumpReaders

    global_template_variables.update({
        "list_vnv_readers": DumpReaders,
        "mongo_configured": Configured
    })

    base.config(config)
    apps.register_blueprint(base.blueprint)
    base.template_globals(global_template_variables)

    base.faker(config.DEFAULT_DATA_PREFIX)

    @apps.context_processor
    def inject_stage_and_region():
        return global_template_variables


def configure_error_handlers(app):
    @app.errorhandler(404)
    def fourohfour(e):
        return render_template('page-404.html'), 404

    @app.errorhandler(403)
    def fourohthree(e):
        return render_template('page-403.html'), 403

    @app.errorhandler(500)
    def fivehundred(e):
        return render_template('page-500.html'), 500


def create_app(config):
    app = Flask(__name__, static_folder='static')
    Markdown(app)

    app.config.from_object(config)
    register_blueprints(app, config)
    configure_error_handlers(app)
    return app



def create_serve_app(config):
    import app.serve

    app = Flask(__name__, static_folder="static")
    app.config.from_object(config)
    socketio = SocketIO(app)
    serve.register(socketio, app, config)

    return socketio, app
