# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from flask import Blueprint, request, url_for, make_response, render_template, flash
from werkzeug.utils import redirect

from app.models.VnVPlugin import VnVPlugin

blueprint = Blueprint(
    'plugins',
    __name__,
    template_folder='templates'
)


def template_globals(globs):
    globs["plugins"] = VnVPlugin.PLUGINS


def faker():
    pass
    #VnVPlugin.add("papi", "/files/ben/papi.so")
    #VnVPlugin.add("dakota", "/files/ben/dakota.so")


@blueprint.route('/new', methods=["POST"])
def new_plugin():
    try:
        plug = VnVPlugin.add(request.form["name"], request.form["filename"])
        return redirect(url_for('base.plugins.view_plugin', id_=plug.id_))
    except Exception as e:
        print(e)
        return make_response("error", 404)


@blueprint.route('/view/<int:id_>')
def view_plugin(id_):
    try:
        plugin = VnVPlugin.find(id_)
        return render_template("plugins/tab-view.html", plugin=plugin)
    except Exception as e:
        print(e)
        flash("Error Finding Plugin")
        return redirect(url_for("base.home"))


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete_plugin(id_):
    VnVPlugin.removeById(id_)
    return "success", 200
