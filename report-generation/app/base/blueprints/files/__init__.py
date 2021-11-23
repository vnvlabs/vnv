# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import os
from flask import Blueprint, make_response, jsonify, flash
from flask import render_template, redirect, url_for, request

from app.models.VnVConnection import VnVLocalConnection, VnVConnection, MAIN_CONNECTION, SetMainConnection, \
    SetFileConnection
from app.rendering.readers import LocalFile
from . import viewers
from app.models.VnVFile import VnVFile
from ...utils.utils import render_error

blueprint = Blueprint(
    'files',
    __name__,
    template_folder='templates'
)
blueprint.register_blueprint(viewers.blueprint, url_prefix="/viewers")


def get_file_template_root():
    sdir = os.path.join(
        os.path.abspath(
            os.path.dirname(__file__)),
        blueprint.template_folder)
    return os.path.abspath(os.path.join(sdir, "renders"))


@blueprint.route('/new', methods=["POST"])
def new():
    try:
        file = VnVFile.add(
            request.form["name"],
            request.form["filename"],
            request.form["reader"],
            get_file_template_root())
        return redirect(url_for("base.files.view", id_=file.id_))
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete(id_):
    refresh = bool(request.args.get("refresh"))
    VnVFile.removeById(id_, refresh is not None)
    if (refresh):
        return make_response(url_for('base.files.view', id_=id_))
    return "success", 200

@blueprint.route("/reader/<int:id_>")
def reader(id_):
    try:
        reader = request.args.get("reader")
        filename = request.args.get("filename", "")
        modal = request.args.get("modal","")
        if id_ != 1000:
            connection = VnVFile.FILES[id_].connection
        else:
            connection = MAIN_CONNECTION()

        render_args = { a[7:]: request.args[a] for a in request.args if a.startswith("render_") }

        if reader == "connection":
            return render_template("files/connection.html", vnvfileid=id_, modal=modal, filename=filename, connection=connection)

        if not connection.connected():
            return render_template("files/connection.html", vnvfileid=id_, modal=modal, filename=filename,
                                   connection=connection, reason="disconnected")

        if len(filename) == 0:
            filename = connection.home()

        if not connection.exists(filename):
            return render_template("files/connection.html", vnvfileid=id_, modal=modal, filename=filename,
                                   connection=connection, reason="does not exist")

        try:

            file = LocalFile(filename, id_, connection, reader=reader, **render_args)
            return render_template("files/browser.html", file=file, modal=modal)

        except Exception as e:
            return render_template("files/browser.html", file=LocalFile(connection.home(), id_, connection, **render_args), error=str(e), modal=modal)

    except Exception as e:
        return render_error(501, "Error Loading File")



@blueprint.route('/close_connection/<int:id_>')
def close_connection(id_):
    if id_ == 1000:
        MAIN_CONNECTION().disconnect()
    else:
        with VnVFile.find(id_) as file:
            file.connection.disconnect()
    return make_response("Ok",200)

@blueprint.route('/open_connection/<int:id_>', methods=["POST"])
def open_connection(id_):

    local = request.form.get('local',False)
    uname = request.form.get("username")
    domain = request.form.get('domain')
    port = int(request.form.get('port'))
    password = request.form.get("password")

    if id_ == 1000:
        r = SetMainConnection(local,uname,domain,password,port)
    else:
        with VnVFile.find(id_) as file:
           r = SetFileConnection(file, local,uname,domain,password,port)

    return make_response("Ok", 200 if r else 201 )


@blueprint.route('/comm/<int:id_>')
def comm(id_):
    with VnVFile.find(id_) as file:
        commId = request.args.get("cid")
        commrender = file.getCommRender(commId)
        return render_template("files/comm.html", commrender=commrender)


@blueprint.route('/data', methods=["GET"])
def data():
    fileId = request.args.get('fileId', type=int)
    nodeId = request.args.get('nodeId')
    with VnVFile.find(fileId) as file:
        return make_response(jsonify(file.getDataChildren(nodeId)), 200)


@blueprint.route('/data_root/<int:id_>', methods=["GET"])
def data_root(id_):
    # One layer of children -- all others just issue children = ""
    node = request.args.get("id", "#")

    if node == "#":
        with VnVFile.find(id_) as file:
            return make_response(jsonify(file.getDataRoot()), 200)
    else:
        return make_response(jsonify([]), 200)


@blueprint.route('/view/<int:id_>')
def view(id_):
    try:
        with VnVFile.find(id_) as file:

            return render_template("files/tab-view.html", file=file)
    except Exception as e:
        print(e)
        return render_error(501, "FIIIIIIIIIIIIIIIIIIIII")


@blueprint.route('/processing/<int:id_>')
def processing(id_):
    try:
        with VnVFile.find(id_) as file:
            if file.isProcessing():
                return make_response("", 200)
            else:
                return make_response("", 201)
    except Exception as e:
        return make_response("", 501)


def unique_files():
    return [
        a for a in VnVFile.FILES.values() if not a.pipeline
    ]


def template_globals(globs):
    viewers.template_globals(globs)
    globs["files"] = VnVFile.FILES
    globs["uniquefiles"] = unique_files


def faker():
    # Development stuff -- this loads some files by default on my computer. Feel free to add your
    # own
    if False and os.path.exists("/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-output"):
        VnVFile.add(
            "test",
            "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-output",
            "json_file",
            get_file_template_root())

    if False and os.path.exists("/home/ben/source/vv/vv-neams/build/examples/dummy/executables/adios-output-live"):
        VnVFile.add(
            "adios",
            "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/adios-output-live",
            "adios_file",
            get_file_template_root())

    if True:
        VnVFile.add(
            "http",
            "5004",
            "json_socket",
            get_file_template_root())


    if False and os.path.exists("/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-output-live"):
        VnVFile.add(
            "test1",
            "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-output-live",
            "json_file",
            get_file_template_root())

