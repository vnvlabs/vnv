# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from flask import Blueprint, render_template
import socketio

from . import blueprints

blueprint = Blueprint(
    'base',
    __name__,
    url_prefix='',
    template_folder='templates'
)

blueprint.register_blueprint(blueprints.plugins.blueprint, url_prefix="/plugins")
blueprint.register_blueprint(blueprints.files.blueprint, url_prefix="/files")
blueprint.register_blueprint(blueprints.inputfiles.blueprint, url_prefix="/inputfiles")
blueprint.register_blueprint(blueprints.notifications.blueprint, url_prefix="/notifications")


@blueprint.route('/')
def default_route():
    return render_template('index.html', segment='index')


@blueprint.route("/")
def home():
    return render_template("index.html", segment="index")

def template_globals(d):
    blueprints.files.template_globals(d)
    blueprints.plugins.template_globals(d)
    blueprints.notifications.template_globals(d)
    blueprints.inputfiles.template_globals(d)


def faker():
    blueprints.files.faker()
    blueprints.plugins.faker()
    blueprints.notifications.faker()
    blueprints.inputfiles.faker()


import jupyter_core.command