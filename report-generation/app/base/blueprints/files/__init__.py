# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import os
import threading

from flask import Blueprint, make_response, jsonify, flash
from flask import render_template, redirect, url_for, request
import socket

import Directory
from app.models.VnVConnection import VnVLocalConnection, VnVConnection, MAIN_CONNECTION, SetMainConnection, \
    SetFileConnection
from app.rendering.readers import LocalFile
from . import viewers
from app.models.VnVFile import VnVFile
from ...utils.mongo import collection_exists
from ...utils.utils import render_error

blueprint = Blueprint(
    'files',
    __name__,
    template_folder='templates'
)
blueprint.register_blueprint(viewers.blueprint, url_prefix="/viewers")


def get_file_template_root():
    return Directory.RENDERS_DIR

def is_port_in_use(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        return s.connect_ex(('0.0.0.0', port)) == 0


class FastReadCounter(object):
    def __init__(self):
        self.value = 0
        self._lock = threading.Lock()

    def increment(self):
        with self._lock:
            self.value += 1
            return self.value - 1

PORTCOUNT = FastReadCounter()
def get_open_port():
    return 14000 + PORTCOUNT.increment()

@blueprint.route('/autostart', methods=["POST"])
def autostart():
    try:
      port = get_open_port()
      if port is None:
          return make_response("error - no available ports",401)

      file = VnVFile.add(
                request.form["name"],
                str(port),
                request.form["reader"],
                get_file_template_root(),
                persist=request.form.get("persist"),
                username=request.form.get("username"),
                password=request.form.get("password"),
                workflowJob = request.form.get("workflowJob",""),
                workflowName=request.form.get("workflowName","")
      )

      return make_response(f'0.0.0.0:{port}',200)
    except Exception as e:
        return make_response("error",501)


@blueprint.route('/new', methods=["POST"])
def new():
    try:

        # If there was a job name, then look for a file with that jobname
        # if it exists, return that
        if "jobName" in request.form:
            file = VnVFile.findByJobName(request.form["jobName"])
            if file is not None:
                return redirect(url_for("base.files.view", id_=file.id_))

        # No jobname so we should load a new file.
        reader = request.form["reader"]
        fname = request.form["filename"]
        if reader == "saved":
            file = VnVFile.add(
                fname,
                fname,
                "mongo",
                get_file_template_root(), persist=True, reload=True)
        else:
            file = VnVFile.add(
                request.form["name"],
                fname,
                reader,
                get_file_template_root(),
                persist=request.form.get("persist"),
                username=request.form.get("username"),
                password=request.form.get("password")
            )

        return redirect(url_for("base.files.view", id_=file.id_))
    except Exception as e:
        return render_error(501, "Error Loading File")



@blueprint.route('/delete/<int:id_>', methods=["POST"])
def delete(id_):
    refresh = bool(request.args.get("refresh"))
    VnVFile.removeById(id_, refresh is not None)
    if (refresh):
        return make_response(url_for('base.files.view', id_=id_))
    return "success", 200

@blueprint.route("/source/<int:id_>/<int:dataId>")
def source(id_,dataId):
    with VnVFile.find(id_) as file:
        d = file.getById(dataId);
        return render_template("files/sourcemap.html", sourcemap = file.getSourceMap(dataId) )

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

@blueprint.route("/workflow/node_image/<string:nodeType>")
def workflow_node_image(nodeType):
    return redirect("/static/assets/images/pink-circle.jpeg")

@blueprint.route("/workflow/render_job/<int:id_>")
def workflow_render_job(id_):
    try:
        with VnVFile.find(id_) as file:
         creator = request.args.get("creator")
         name = request.args.get("name")
         code = request.args.get("code")
         if creator is not None:
            c = creator.split(":")
            return make_response(file.render_workflow_job(c[0],c[1], code),200)

    except Exception as e:
        print(e)
    return make_response("",200)

@blueprint.route("/workflow/raw_rst/<int:id_>")
def workflow_render_rst(id_):
    try:
        with VnVFile.find(id_) as file:
         creator = request.args.get("creator")
         if creator is not None:
            c = creator.split(":")
            return make_response(file.render_workflow_rst(c[0],c[1], request.args.get("jobName")),200)

    except Exception as e:
        print(e)
    return make_response("",200)

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
            if "full" in  request.args:
                return render_template("files/tab-view-content.html", file=file, count=int(request.args.get("count","0")))
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
        a for a in VnVFile.FILES.values()
    ]


def template_globals(globs):
    viewers.template_globals(globs)
    globs["files"] = VnVFile.FILES
    globs["uniquefiles"] = unique_files

files = [
    [True, "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-output", "json_file",{"persist": True}],
    [True, "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-workflow-output", "json_file", {"persist":True}],
    [False, "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/adios-output-live", "adios", True, {}],
    [False, "/home/ben/source/vv/vv-neams/build/examples/dummy/executables/vv-output-live", "json_file",{"persist": True}],
    [False, "/home/ben/source/vv/applications/asgard/build/vv-output", "json_file",{"persist": True}],
]

def faker():
    # Development stuff -- this loads some files by default on my computer. Feel free to add your
    # own
  for f in files:
    if f[0] and os.path.exists(f[1]):
        VnVFile.add("faker", f[1], f[2], get_file_template_root(), **f[3])


