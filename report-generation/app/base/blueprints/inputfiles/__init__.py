# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""
import glob
import json
import os
import uuid
from pathlib import Path

from flask import Blueprint, make_response, jsonify
from flask import render_template, redirect, url_for, request

from app.Directory import STATIC_FILES_DIR
from app.models import VnVFile
from app.models.VnVInputFile import VnVInputFile
from ..files import get_file_from_runinfo
from ...utils import mongo
from ...utils.utils import render_error

vnv_executables = {}
vnv_plugins = {}

blueprint = Blueprint(
    'inputfiles',
    __name__,
    template_folder='templates',
    url_prefix="/inputfiles"
)


@blueprint.route('/new', methods=["POST"])
def new():
    try:

        c = request.form.get("executable")
        if c == "Custom":
            path = request.form["path"]
            defs = {}
        else:
            path = os.path.join(VnVInputFile.VNV_PREFIX, vnv_executables.get(c)[0])
            defs = vnv_executables.get(c)[2]

        file = VnVInputFile.add(request.form["name"], path, defs=defs)

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
        file.saveInput(form["value"])
        return make_response("", 200)


@blueprint.route('/save_exec/<int:id_>', methods=["POST"])
def save_exec(id_):
    with VnVInputFile.find(id_) as file:
        form = request.get_json()
        file.exec = form["value"]
        return make_response("", 200)

@blueprint.route('/save-issues/<int:id_>', methods=["POST"])
def save_issues(id_):
    try:
        with VnVInputFile.find(id_) as file:
            if "data" in request.args:
                try:
                    a = json.loads(request.args["data"])
                    file.issues = json.dumps(a)
                    return make_response("", 200)
                except:
                    return make_response("", 201)
            return make_response("", 202)
    except:
        return make_response("", 203)


@blueprint.route('/save_psip/<int:id_>', methods=["POST"])
def save_psip(id_):
    try:
        with VnVInputFile.find(id_) as file:
            if "data" in request.args:
                try:
                    a = json.loads(request.args["data"])
                    file.psip = request.args["data"]
                    return make_response("", 200)
                except:
                    return make_response("", 201)
            return make_response("", 202)
    except:
        return make_response("", 203)

@blueprint.route('/toggle_psip/<int:id_>', methods=["POST"])
def enable_psip(id_):
    try:
        with VnVInputFile.find(id_) as file:
            file.psip_enabled = not file.psip_enabled
            return make_response("show" if file.psip_enabled else "hide", 200)
    except:
        return make_response("", 203)

@blueprint.route('/toggle_issues/<int:id_>', methods=["POST"])
def enable_issues(id_):
    try:
        with VnVInputFile.find(id_) as file:
            file.issues_enabled = not file.issues_enabled
            return make_response("show" if file.issues_enabled else "hide", 200)
    except:
        return make_response("", 203)


@blueprint.route('/validate/<string:comp>/<int:id_>', methods=["POST"])
def validate_input(comp, id_):
    with VnVInputFile.find(id_) as file:
        form = request.get_json()
        if comp == "inputfile":
            r = file.validateInput(form["value"]);
        elif comp == "execfile":
            r = file.validateExecution(form["value"])
        else:
            r = []

        return make_response(jsonify(r), 200 if len(r) == 0 else 201)


@blueprint.route('/load_input/<int:id_>', methods=["POST"])
def load_input(id_):
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


@blueprint.route('/dependency/new/<int:fileid_>', methods=["POST"])
def newdep(fileid_):
    with VnVInputFile.find(fileid_) as file:
        deps = file.add_dependency(remoteName="NEW DEP",type="NEW DEP")
        return render_template("inputfiles/deps.html", deps=deps)

@blueprint.route('/dependency/delete/<int:fileid_>/<depId>', methods=["POST"])
def deletedep(fileid_, depId):
    with VnVInputFile.find(fileid_) as file:
        deps = file.delete_dependency(depId)
        return render_template("inputfiles/deps.html", deps=deps)

        return make_response("FFF",200)

@blueprint.route('/dependency/edit/<int:fileid_>', methods=["POST"])
def editdep(fileid_):

    print(request.form)
    print(request.files)

    with VnVInputFile.find(fileid_) as file:
        depId = request.form["depId"]
        depType = request.form["depType"]
        kw = {}
        if depType == "upload":
            fname = os.path.join(STATIC_FILES_DIR,uuid.uuid4().hex)
            if "file" in request.files and request.files["file"] is not None:
                request.files["file"].save(fname)
                kw["text"] = fname
                kw["original"] = request.files["file"].filename
        else:
            kw["text"] = request.form["text"]

        if "hive" in request.form:
            kw["hive"] = request.form["hive"]

        if depId is None or len(depId) == 0:
            deps = file.add_dependency(remoteName=request.form["rname"], type=request.form["depType"], **kw )
        else:
            deps = file.edit_dependency(depId, remoteName=request.form["rname"], type=request.form["depType"], **kw)

        return render_template("inputfiles/deps.html", deps=deps)


@blueprint.route('/dependency/get/<int:fileid_>', methods=["POST"])
def getdep(fileid_):

    with VnVInputFile.find(fileid_) as file:
        dep = file.deps.get(request.args.get("depId"))
        if dep is not None:
            return make_response(jsonify(dep.to_json()),200)
    return make_response("Error",204)


@blueprint.route('/dependency/view/<int:fileid_>/<depId>', methods=["POST"])
def viewdep(fileid_, depId):
    with VnVInputFile.find(fileid_) as file:
        return make_response("FFF",200)



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
            r = getattr(file, "autocomplete_" + comp)(row, col, pre, val, plugins=vnv_plugins)
            return make_response(jsonify(r), 200)

    return make_response(jsonify([]), 200)


@blueprint.route('/moose/autocomplete', methods=["POST"])
def moose_auto():
    j = request.get_json()
    fileId = j["fileId"]
    depId = j["depId"]
    row = j["row"]
    col = j.get('col',0)
    pre = j["pre"]
    val = j["value"]
    dump = j["dump"]
    with VnVInputFile.find(fileId) as file:
        return make_response(jsonify(file.autocomplete_moose(depId, dump, row,col,pre,val)),200)

    return make_response(jsonify([]), 200)



@blueprint.route('/moose/validate', methods=["POST"])
def moose_validate():
    j = request.get_json()

    depId = j["depId"]
    fileId = j["fileId"]
    value = j["value"]
    dump = j["dump"]

    with VnVInputFile.find(fileId) as file:
        return make_response(jsonify(file.validate_moose(depId, dump, value)),200)

    return make_response(jsonify([]), 200)


@blueprint.route('/update_main_header/<int:id_>')
def main_header(id_):
    with VnVInputFile.find(id_) as file:
        data = {
            "header": render_template("inputfiles/main_header_content.html", file=file),
            "spec": file.spec,
            "desc": file.get_executable_description()
        }
        return make_response(jsonify(data), 200)
    return make_response("Error", 500)


@blueprint.route('/get_spec/<int:id_>')
def get_spec(id_):
    with VnVInputFile.find(id_) as file:
        return make_response(file.spec, 200)


@blueprint.route('/autocomplete')
def autocomplete():
    pref = request.args.get('prefix', '')
    file = request.args.get("file")
    try:
        with VnVInputFile.find(int(file)) as f:
            return make_response(jsonify(f.connection.autocomplete(pref)), 200)
    except:
        make_response(jsonify(glob.glob(pref + "*")), 200)


@blueprint.route('/get_desc/<int:id_>')
def get_desc(id_):
    with VnVInputFile.find(id_) as file:
        return make_response(file.get_executable_description(), 200)


@blueprint.route('/configure/<int:id_>', methods=["POST"])
def configure(id_):
    with VnVInputFile.find(id_) as file:

        if "local" in request.form:
            file.setConnectionLocal()
        else:
            username = request.form["username"]
            port = request.form["port"]
            domain = request.form["domain"]
            password = request.form["password"]
            file.setConnection(domain, username, password, port)

        file.setFilename(request.form.get("application", file.filename), request.form.get("specDump", file.specDump))
        return render_template("inputfiles/connection_content.html", file=file)


@blueprint.route('/view/<int:id_>')
def view(id_):
    try:
        with VnVInputFile.find(id_) as file:
            return render_template("inputfiles/view.html", file=file)
    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


@blueprint.route('/joblist/<int:id_>')
def joblist(id_):
    try:
        with VnVInputFile.find(id_) as file:
            return render_template("inputfiles/joblist.html", file=file)
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/delete_job/<int:id_>/<jobid>', methods=["POST"])
def delete_job(id_, jobid):
    with VnVInputFile.find(id_) as file:
        file.connection.delete_job(jobid);
        return render_template("inputfiles/joblist.html", file=file)
    return render_error(401, "Huh")


@blueprint.route('/cancel_job/<int:id_>/<jobid>', methods=["POST"])
def cancel_job(id_, jobid):
    with VnVInputFile.find(id_) as file:
        file.connection.cancel_job(jobid);
        res = render_template("inputfiles/joblist.html", file=file)
        return res
    return render_error(401, "Huh")


@blueprint.route('/openreport/<int:id_>')
def openreport(id_):
    try:
        with VnVInputFile.find(id_) as file:
            pref = os.path.join(request.args["dir"], "vnv_" + request.args["id"] + "_")
            reports = file.connection.autocomplete(pref)

            if "confirmed" in request.args:
                for i in reports:
                    with open(file.connection.download(i), 'r') as ff:
                        ff = get_file_from_runinfo(json.load(ff))
                        return make_response(url_for('base.files.view', id_=ff.id_),200)

            return make_response("Error", 201)

    except Exception as e:
        print(e)
        return make_response("Error", 201)


@blueprint.route('/delete-all', methods=["POST"])
def delete_all():
    VnVInputFile.delete_all()
    return make_response("Complete", 200)


@blueprint.route('/update_display_name/<int:id_>', methods=["POST"])
def update_display_name(id_):
    with VnVInputFile.find(id_) as file:
        file.displayName = request.args.get("new", file.displayName)
        return make_response(file.displayName, 200)


@blueprint.route('/execute/<int:id_>')
def execute(id_):
    try:
        with VnVInputFile.find(id_) as file:
            if "dryrun" in request.args:
                script, name = file.script(request.args.get("val", ""),"SAMPLE")
                return make_response(f'''Job Name: {name}\n\n{script}''', 200)
            else:
                return make_response(file.execute(request.args.get("val", "")), 200)

    except Exception as e:
        print(e)
        return render_error(501, "Error Loading File")


def list_vnv_executables():
    return [[k, v[1]] for k, v in vnv_executables.items()]


def template_globals(globs):
    globs["inputfiles"] = VnVInputFile.FILES
    globs["list_vnv_executables"] = list_vnv_executables


def faker(PREFIX):
    VnVInputFile.loadAll()
    VnVInputFile.VNV_PREFIX = PREFIX
