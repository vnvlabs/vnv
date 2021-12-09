# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import json
from pathlib import Path

from flask import Blueprint, make_response
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

        file = VnVInputFile.add(
            request.form["name"]
        )
        return redirect(url_for("base.inputfiles.view", id_=file.id_))
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete(id_):
    VnVInputFile.removeById(id_)
    return "success", 200


@blueprint.route('/disconnect/<int:id_>', methods=["POST"])
def disconnect(id_):
    with VnVInputFile.find(id_) as file:
        file.connection.disconnect();
        return render_template("inputfiles/connection_content.html", file=file)


@blueprint.route('/load_input_file/<int:id_>', methods=["POST"])
def load_input_file(id_):
    path = request.form["filename"]
    with VnVInputFile.find(id_) as file:
        file.loadfile = path

        if not file.connection.connected():
            return make_response("Please open a connection before continuing", 201)

        if file.connection.exists(path):
            if file.connection.is_dir(path):
                return make_response("Cannot load a directory", 201)
            try:
                p = file.connection.download(path)
                txt = Path(p).read_text()
                return make_response(txt, 200)
            except Exception as e:
                return make_response(f"Something went wrong ({e}", 202)
        else:
            return make_response("File does not exist", 200)


@blueprint.route('/save_input_file/<int:id_>', methods=["POST"])
def save_input_file(id_):
    file = VnVInputFile.find(id_)
    file.value = request.form["value"]
    return make_response("", 200)


@blueprint.route('/load_spec/<int:id_>', methods=["POST"])
def load_spec(id_):
    with VnVInputFile.find(id_) as file:
        path = request.form["filename"]
        file.specDump = path

        if not file.connection.connected():
            return make_response("Please open a connection before continuing", 201)

        try:
            file.loadSpec()
            return make_response(file.spec, 200)
        except Exception as e:
            return make_response(f"Something went wrong ({e}", 202)


@blueprint.route('/save_spec/<int:id_>', methods=["POST"])
def save_spec(id_):
    with VnVInputFile.find(id_) as file:
        file.spec = request.form["value"]
        return make_response("", 200)


@blueprint.route('/load_exec/<int:id_>', methods=["POST"])
def load_exec(id_):
    path = request.form["filename"]
    with VnVInputFile.find(id_) as file:
        file.execFile = path

        if not file.connection.connected():
            return make_response("Please open a connection before continuing", 201)

    if file.connection.exists(path):
        if file.connection.is_dir(path):
            return make_response("Cannot load a directory", 201)
        try:
            p = file.connection.download(path)
            txt = Path(p).read_text()
            return make_response(txt, 200)
        except Exception as e:
            return make_response(f"Something went wrong ({e}", 202)
    else:
        return make_response("File does not exist", 200)


@blueprint.route('/save_exec/<int:id_>', methods=["POST"])
def save_exec(id_):
    with VnVInputFile.find(id_) as file:
        file.exec = request.form["value"]
        return make_response("", 200)


@blueprint.route('/connected/<int:id_>', methods=["GET"])
def connected(id_):
    with VnVInputFile.find(id_) as file:
        return make_response("", 200 if file.connection.connected() else 201)


@blueprint.route('/update_main_header/<int:id_>')
def main_header(id_):
    with VnVInputFile.find(id_) as file:
        return render_template("inputfiles/main_header_content.html", file=file)


@blueprint.route('/configure/<int:id_>', methods=["POST"])
def configure(id_):
    with VnVInputFile.find(id_) as file:

        if not file.connection.connected():
            if "local" in request.form:
                file.setConnection()
            else:
                username = request.form["username"]
                port = request.form["port"]
                domain = request.form["domain"]
                password = request.form["password"]
                file.setConnection(domain, username, password, port)

        file.filename = request.form.get("application", file.filename)
        return render_template("inputfiles/connection_content.html", file=file)


@blueprint.route('/view/<int:id_>')
def view(id_):
    try:
        with VnVInputFile.find(id_) as file:
            return render_template("inputfiles/view.html", file=file)
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


def template_globals(globs):
    globs["inputfiles"] = VnVInputFile.FILES


def faker():
    VnVInputFile.add("example")
