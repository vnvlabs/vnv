import json

import jsonschema
from flask import Blueprint, request, render_template, jsonify, make_response

from app.base.utils.utils import render_error
from app.models import VnVFile
from app.rendering.vnvdatavis.directives.dataclass import DataClass

blueprint = Blueprint(
    'viewers',
    __name__,
    url_prefix='/viewers',
    template_folder='templates'
)


def template_globals(globs):
    globs["viewers"] = ["processor", "communicator", "graph"]


@blueprint.route('/render/<int:id_>')
def render(id_):
    try:
        with VnVFile.VnVFile.find(id_) as file:
            return render_template("viewers/view.html", file=file)

    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/package/<int:id_>')
def package(id_):
    try:
        with VnVFile.VnVFile.find(id_) as file:
            pack = request.args.get("p")
            return file.render_package(pack)
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/action/<int:id_>')
def action(id_):
    try:
        with VnVFile.VnVFile.find(id_) as file:
            pack = request.args.get("p")
            return file.render_action(pack)
    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/unit/<int:id_>')
def unit(id_):
    try:

        utest = request.args.get("uid", type=int)
        test = request.args.get("uuid", type=str)
        with VnVFile.VnVFile.find(id_) as file:
            return file.render_unit_test(utest, test)

    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/unit_table/<int:id_>')
def unit_table(id_):
    with VnVFile.VnVFile.find(id_) as file:
        return make_response(file.unit_test_table(), 200)


@blueprint.route('/respond', methods=["POST"])
def respond():
    try:
        fileid = int(request.form.get('fileid'))
        ipid = int(request.form.get('ipid'))
        id_ = int(request.form.get("id"))
        jid = int(request.form.get("jid"))
        response = request.form.get("response")

        with VnVFile.VnVFile.find(fileid) as file:
            return file.respond(ipid, id_, jid, response)

        return make_response("Got It", 200)

    except Exception as e:
        return render_error(501, "Error Loading File")


@blueprint.route('/validateResponse/<int:id_>/<int:ipid>', methods=["POST"])
def validateResponse(id_, ipid):
    try:
        with VnVFile.VnVFile.find(id_) as file:
            iprender = file.render_ip(ipid)
            r = iprender.getRequest()
            req = json.loads(request.get_json().get('data'))
            sch = r.getSchema()
            jsonschema.validate(req, sch)
            return make_response("Valid", 200)
    except Exception as e:
        return make_response("Invalid", 201)


@blueprint.route('/iprequest/<int:id_>/<int:ipid>')
def iprequest(id_, ipid):
    try:
        with VnVFile.VnVFile.find(id_) as file:

            iprender = file.render_ip(ipid)
            r = iprender.getRequest()
            if r is None:
                s = iprender.getStatus()
                return make_response("", 202 if s == "Complete" else 201)
            else:
                return render_template("viewers/requestRender.html", iprender=iprender, requestRender=r)
    except Exception as e:
        return make_response("Error", 200)


@blueprint.route('/processing/<int:id_>/<int:ipid>')
def processing(id_, ipid):
    try:
        with VnVFile.VnVFile.find(id_) as file:

            iprender = file.render_ip(ipid)
            r = iprender.getStatus()
            return make_response(r, 201 if r == "Complete" else 200)

    except Exception as e:
        return make_response("Error", 200)


def rst_string_to_html(content):
    return "<div></div>"


@blueprint.route("/rst/render")
def rst():
    try:

        id_ = int(request.args["fileId"])
        dataId = int(request.args["adataId"])
        content = request.args["content"]


        with VnVFile.VnVFile.find(id_) as file:
            data = file.getById(dataId).cast() if dataId is not None else file.root
            templateName = file.render_temp_string(content)

            config = render_template(templateName, data=DataClass(data, data.getId(), id_))
            return render_template("viewers/rst_render.html" , content = config );

    except Exception as e:
        return make_response("<h1>Error</h1>", 200)


@blueprint.route("/dataexplorer/<int:fileId>/<int:dataId>")
def dataexplorer(fileId,dataId):
    return render_template("viewers/data.html", fileId=fileId, dataId=dataId)

@blueprint.route("/rst/raw/<int:id_>/<int:dataId>")
def rstraw(id_,dataId):
    try:

        with VnVFile.VnVFile.find(id_) as file:
            data = file.getById(dataId).cast()
            content = file.templates.get_raw_rst(data)
            return make_response(content, 200)
    except Exception as e:
        return make_response("Error\n-----", 200)


@blueprint.route('/ip/<int:id_>')
def ip(id_):
    try:
        with VnVFile.VnVFile.find(id_) as file:
            injection = request.args.get('ipid', type=int)
            iprender = file.render_ip(injection)

            if isinstance(iprender, str):
                return iprender
            resp = render_template(
                "viewers/injectionPoint.html",
                iprender=iprender)

            return resp

    except Exception as e:
        return render_error(501, "Error Loading File")

@blueprint.route('/render_label/<int:id_>')
def render_label(id_):
    try:
        injection = request.args.get('ipid', type=int)
        if injection < 0 :
            return "<h4>Main Application</h4>"

        with VnVFile.VnVFile.find(id_) as file:

            iprender = file.render_ip(injection)
            return render_template("general/label.html", iprender=iprender)
    except Exception as e :
        return make_response("error",200)


@blueprint.route('/data_root/<int:id_>', methods=["GET"])
def data_root(id_):
    node = request.args.get("ipid", type=int)
    with VnVFile.VnVFile.find(id_) as file:
        node = file.getById(node).cast()
        return make_response(jsonify([{
            "text": node.getName(),
            "children": True,
            "li_attr": {
                "fileId": file.id_,
                "nodeId": node.getId(),
            }
        }]), 200)

    return make_response(jsonify([]), 200)


@blueprint.route('/jmes-query', methods=["GET"])
def query():
    id_ = int(request.args.get('fileId'))
    dataid = int(request.args.get('dataId'))
    quer = request.args.get('query', "")
    try:
        with VnVFile.VnVFile.find(id_) as file:
            d = file.query_str(dataid, quer)
            return make_response(d, 200)
    except:
        return make_response(jsonify("error"), 200)


@blueprint.route("next")
def next():
    id_ = request.args.get("id_", type=int)
    count = request.args.get("count", 100, type=int)
    try:
        with VnVFile.VnVFile.find(id_) as file:
            data = file.proc_iter_next(count)
            endtime = file.root.getEndTime()
            return jsonify({"data": data, "endtime" : endtime}), 200
    except BaseException as e:
        return render_error(501, "Error Loading File")
