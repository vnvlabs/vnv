# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import json
import os
from flask import Blueprint
from flask import render_template, redirect, url_for, request

from app.models.VnVInputFile import VnVInputFile
from ...utils.utils import render_error

blueprint = Blueprint(
    'inputfiles',
    __name__,
    template_folder='templates'
)


@blueprint.route('/new', methods=["POST"])
def new():
    try:
        load = request.form.get('load')
        if load is not None:
            defVal = json.load(load)
        else:
            defVal = {}

        file = VnVInputFile.add(
            request.form["name"],
            request.form["filename"],
            defVal)
        return redirect(url_for("base.inputfiles.view", id_=file.id_))
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete(id_):
    VnVInputFile.removeById(id_)
    return "success", 200


@blueprint.route('/view/<int:id_>')
def view(id_):
    try:
        file = VnVInputFile.find(id_)
        return render_template("inputfiles/view.html", file=file)
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


def template_globals(globs):
    globs["inputfiles"] = VnVInputFile.FILES
