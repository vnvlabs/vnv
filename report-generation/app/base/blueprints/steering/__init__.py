# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import json
import os
from flask import Blueprint
from flask import render_template, redirect, url_for, request

from app.models.VnVSteering import VnVSteering
from ...utils.utils import render_error

blueprint = Blueprint(
    'steering',
    __name__,
    template_folder='templates'
)


@blueprint.route('/new', methods=["POST"])
def new():
    try:
        file = VnVSteering.add(request.form["name"], request.form["application"])
        return redirect(url_for("base.steering.view", id_=file.id_))
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete(id_):
    VnVSteering.removeById(id_)
    return "success", 200


@blueprint.route('/view/<int:id_>')
def view(id_):
    try:
        file = VnVSteering.find(id_)
        return render_template("steering/view.html", file=file)
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


@blueprint.route('/pending/<int:id_>')
def pending(id_):
    try:
        file = VnVSteering.find(id_)
        p = request.args.get('p')
        return render_template("steering/render.html", steer=file.render(p))
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


@blueprint.route('/respond', methods=["POST"])
def respond():
    try:
        id_ = request.form.get("id")
        fileId = request.form.get("fileId")
        file = VnVSteering.find(int(fileId))
        file.respond(id_, request.form.get("response"))
        return redirect(url_for('base.steering.view', id_=fileId))
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


def template_globals(globs):
    globs["steerings"] = VnVSteering.FILES
