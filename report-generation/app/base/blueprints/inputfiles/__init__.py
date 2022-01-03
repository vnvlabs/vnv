# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import json
from pathlib import Path

from flask import Blueprint, make_response, jsonify
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




@blueprint.route('/save_input/<int:id_>', methods=["POST"])
def save_input_file(id_):
    with VnVInputFile.find(id_) as file:
        form = request.get_json()
        file.value = form["value"]
        return make_response("", 200)

@blueprint.route('/save_spec/<int:id_>', methods=["POST"])
def save_spec(id_):
    with VnVInputFile.find(id_) as file:
        form = request.get_json()
        file.spec = form["value"]
        return make_response("", 200)

@blueprint.route('/save_exec/<int:id_>', methods=["POST"])
def save_exec(id_):
    with VnVInputFile.find(id_) as file:
        form = request.get_json()
        file.exec = form["value"]
        return make_response("", 200)

@blueprint.route('/validate/<string:comp>/<int:id_>', methods=["POST"])
def validate_input(comp, id_):
    with VnVInputFile.find(id_) as file:
        form = request.get_json()
        if comp == "inputfile":
            r = file.validateInput(form["value"]);
        elif comp == "execfile":
            r = file.validateExecution(form["value"])
        elif comp == "specfile":
            r = file.validateSpec(form["value"])
        else:
            r = []

        return make_response(jsonify(r),  200 if len(r) == 0 else 201 )




@blueprint.route('/load_input/<int:id_>', methods=["POST"])
def load_input(id_):
    return make_response("sdfsdf", 200)
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


@blueprint.route('/load_spec/<int:id_>', methods=["POST"])
def load_spec(id_):
    return make_response("esedfsdf", 200)

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




@blueprint.route('/load_exec/<int:id_>', methods=["POST"])
def load_exec(id_):
    return make_response("sadfsdf", 200)

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


@blueprint.route('/connected/<int:id_>', methods=["GET"])
def connected(id_):
    with VnVInputFile.find(id_) as file:
        return make_response("", 200 if file.connection.connected() else 201)


@blueprint.route('/autocomplete/<string:comp>/<int:id_>', methods=["GET"])
def input_autocomplete(comp, id_):

    row = request.args["row"]
    col = request.args["col"]
    pre = request.args["pre"]
    val = request.args["val"].split("\n")
    with VnVInputFile.find(id_) as file:
        if hasattr(file, "autocomplete_" + comp):
           r = getattr(file,"autocomplete_" + comp)(row,col,pre,val)
           return make_response(jsonify(r), 200 )

    return make_response(jsonify([]), 200)


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
