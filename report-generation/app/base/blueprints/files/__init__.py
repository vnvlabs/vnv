# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import hashlib
import os
import pathlib
import uuid

import flask
import pygments
from flask import Blueprint
from flask import render_template, redirect, url_for, request
from pygments.lexers import guess_lexer_for_filename

from urllib.request import pathname2url

from app.rendering.readers import render_reader
from . import viewers
from app.models.VnVFile import VnVFile
from ...utils.utils import render_error

blueprint = Blueprint(
    'files',
    __name__,
    template_folder='templates'
)
blueprint.register_blueprint(viewers.blueprint, url_prefix="/viewers")



def get_file_template_dir(name):
    c = 0
    sdir = os.path.join(os.path.abspath(os.path.dirname(__file__)), blueprint.template_folder)
    while pathlib.Path(os.path.join(sdir, "renders", f'{name}_{c}')).exists():
        c = c + 1
    return os.path.abspath(os.path.join(sdir, "renders", f'{name}_{c}'))


@blueprint.route('/new', methods=["POST"])
def new():
    try:
        file = VnVFile.add(request.form["name"], request.form["filename"], request.form["reader"],
                           get_file_template_dir(request.form["name"]))
        return redirect(url_for("base.files.view", id_=file.id_))
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete(id_):
    VnVFile.removeById(id_)
    return "success", 200

@blueprint.route("/reader")
def reader():
    reader = request.args.get("reader", "auto")
    filename = request.args.get("filename");

    if os.path.exists(filename):
        return render_reader(filename, reader)
    else:
        return render_error(501, "Filename does not exist")

@blueprint.route('/comm/<int:id_>')
def comm(id_):
    with VnVFile.find(id_) as file:
        commId = request.args.get("cid")
        commrender = file.getCommRender(commId)
        return render_template("files/comm.html", commrender=commrender)


@blueprint.route('/view/<int:id_>')
def view(id_):
    try:
        with VnVFile.find(id_) as file:
            return render_template("files/tab-view.html", file=file)
    except Exception as e:
        return render_error(501, "Error Loading File")





def template_globals(globs):
    viewers.template_globals(globs)
    globs["files"] = VnVFile.FILES


def faker():
    pass
    VnVFile.add("test", "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-output", "json_file",
                get_file_template_dir("test"))
    # VnVFile.add("sample2", "/files/ben/sample1.json", "json_file", "./sdfsd")
    # VnVFile.add("sample3", "/files/ben/sample1.json", "json_file", "./sdfsd")
