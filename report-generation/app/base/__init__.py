# -*- encoding: utf-8 -*-
import glob
import os
from pathlib import Path
from flask import Blueprint, render_template, request, make_response, jsonify

from . import blueprints

blueprint = Blueprint(
    'base',
    __name__,
    url_prefix='',
    template_folder='templates'
)

blueprint.register_blueprint(
    blueprints.plugins.blueprint,
    url_prefix="/plugins")
blueprint.register_blueprint(blueprints.files.blueprint, url_prefix="/files")
blueprint.register_blueprint(
    blueprints.inputfiles.blueprint,
    url_prefix="/inputfiles")

blueprint.register_blueprint(
    blueprints.notifications.blueprint,
    url_prefix="/notifications")

blueprint.register_blueprint(
    blueprints.directives.blueprint,
    url_prefix="/directives")

@blueprint.route('/')
def default_route():
    return render_template('index.html', segment='index')


@blueprint.route("/")
def home():
    return render_template("index.html", segment="index")


@blueprint.route('/autocomplete')
def autocomplete():
    pref = request.args.get('prefix', '')
    #p = os.path.join(Path.home(), pref)
    # print(p)

    return make_response(jsonify(glob.glob(pref + "*")), 200)


def template_globals(d):
    blueprints.files.template_globals(d)
    blueprints.plugins.template_globals(d)
    blueprints.notifications.template_globals(d)
    blueprints.inputfiles.template_globals(d)
    blueprints.directives.template_globals(d)


def faker():
    blueprints.files.faker()
