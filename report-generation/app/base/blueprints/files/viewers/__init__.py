import json
import re

import jsonschema
from flask import Blueprint, request, render_template, jsonify, make_response, render_template_string

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
    globs["viewers"] = ["processor", "communicator"]

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
            if iprender is None:
                return make_response("Invalid", 201)

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
            if iprender is None:
                make_response("Error", 200)

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
            if iprender is None:
                make_response("Error", 200)

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
            templateName = file.render_to_string(content)
            config = render_template_string(templateName, data=DataClass(data, data.getId(), id_))
            return render_template("viewers/rst_render.html", content=config);

    except Exception as e:
        return make_response("<h1>Error</h1>", 200)


@blueprint.route("/dataexplorer/<int:fileId>/<int:dataId>")
def dataexplorer(fileId, dataId):
    return render_template("viewers/data.html", fileId=fileId, dataId=dataId)


# TODO Extend this support -- infrastructure is their.
def listDirectives():
    return [
        ["chart", "", "{}",[]],
        ["plotly-raw", "", "{}",[]],
        ["table","","{}",[]],
        ["apex","","{}",[]],
        ["gchart","","{}",[]],
        ["treechart","",{},[]],
        ["terminal","","ls -l",[]],
        ["plotly-trace","", "", [":trace.data: bar",":data.x: [\"a\" , \"b\"]", ":data.y: [1,2]" ]]
    ]


def getDirectiveOptions(directive, ops):
    if ":" in ops:
        return [] # TODO could supply enum values here if we want.

    # TODO fetch matching option names.
    return [{"caption": f'{ops}{k}', "value": f'{ops}{k}: ', "meta": "option value"} for k in range(0, 4)]


def getDirectiveArgs(directive, prefix, args):
    return [] #TPDP


def getDirectiveBody(directive, val, row, col, start, end ):
    return [] #TODO


def getDirectiveCompletions(id_, dataId, prefix, value):
    return [] #TODO


def getVnVRole(id_, dataId, role, val):
    return [] #TODO

rst_list = []
rst_rlist = []

jj = ["=", "-", "^", "\""]
rst_rlist.append({"caption": "vnv", "value": ":vnv:``", "meta": "role"})
rst_rlist.append({"caption": "strong", "value": "**<txt>**", "meta": "role"})
rst_rlist.append({"caption": "emphasis", "value": "*<txt>*", "meta": "role"})
rst_rlist.append({"caption": "literal", "value": "``<txt>``", "meta": "role"})
rst_rlist.append({"caption": "itemize", "value": "\n\n- Item 1\n- Item 2\n-Item 3\n\n", "meta": "directive"})
rst_rlist.append({"caption": "enumerate", "value": "\n\n#. Item 1\n#. Item 2\n#. Item 3\n\n", "meta": "directive"})
rst_rlist.append({"caption": "table",
             "value": '''\n\n=====  =====  ======\n   Inputs     Output\n------------  ------\n  A      B    A or B\n=====  =====  ======\nFalse  False  False\nTrue   False  True\n=====  =====  ======\n\n''',
             "meta": "directive"})


rst_list.append({"caption": "image", "value": "image:: <url>\n  :width: 300px\n  :height: 300px\n  :align: true\n\n", "meta": "directive"})
rst_list.append({"caption": "figure", "value": "figure:: <url>\n  :width: 300px\n  :height: 300px\n  :align: true\n\n  This is the caption (must be indented 2 spaces)\n\n", "meta": "directive"})

gg = ["tip","note","important","error","danger","warning","caution"]
for i in gg:
    rst_list.append({"caption": i, "value": f"{i}:: This is the content\n\n", "meta": "directive"})

for i in listDirectives():
    d = {"caption": "vnv-" + i[0], "meta": "vnv", "value": f"vnv-{i[0]}:: {i[1]}\n"}
    for ii in i[3]:
        d["value"] += f"  {ii}\n"
    d["value"] += f"\n  {i[2]}\n\n"
    rst_list.append(d)

def getRoleList(col):
    r = [{"caption": f"h{i}", "value": "\n" + f'{jj[i]}' * col, "meta": "header"} for i in range(0, 3)]
    return r + rst_rlist


def getDirectiveList(prefix):
    return [ {"caption" : i["caption"], "value" : prefix + i["value"], "meta" : i["meta"]}  for i in rst_list]


def getDirectiveOption(directive, option, value, pre):
    return [] #Todo

def getDirectiveOptionNames(directive, pre):
    return [] #Todo


def getAllRoles():
   return [
       {"caption" : "vnv", "value" : "vnv:`Name`", "meta" : "vnv" }
   ]


@blueprint.route("/rst/auto/<int:id_>/<int:dataId>")
def rstauto(id_, dataId):
    try:
        row = int(request.args.get("row", 0))
        col = int(request.args.get("col", 0))
        val = request.args.get("val", "").split("\n")
        pre = request.args.get("prefix","")

        # if we are inside a role:
        r = col-1
        v = val[row]
        while r > 3:
            if (v[r]) == "`":
                if v[r-1] != ":":
                    break
                b = r-2
                while b >= 0:
                    if v[b] in [" ","`"]:
                        b = -1
                    elif v[b] == ":":
                        return make_response(jsonify(getVnVRole(id_,dataId, v[b+1:r-1],v[(r+1):])),200)
                    else:
                        b-=1
            else:
                r-=1

        # if it is an option for a directive (inline option)
        reg = re.compile(".. (.*?)::(.*)$")
        m = re.match(reg,val[row][0:col])
        if m:
            op = m.group(2).lstrip().split(" ")
            return make_response(jsonify(getDirectiveArgs(m.group(1),pre, op)),200)


        # Check if its an option.

        #match an option
        rt = re.compile('^ [ +]:(.*)?: (.*)$')
        mt = re.match(rt,val[row][0:col])
        if mt:
            b = row
            while b > 0 and val[b].lstrip().startswith(":"):
                b-=1
            mm = re.match(reg,val[b])
            if mm:
                directive = mm.group(1)
                return make_response(jsonify(getDirectiveOption(directive,mt.group(1), mt.group(2),pre)),200)

        # match a half finished option
        rt = re.compile('^ [ +]:([a-zA-Z0-9_-]*$)')
        mt = re.match(rt, val[row][0:col])
        if mt:
            b = row
            while b > 0 and val[b].lstrip().startswith(":"):
                b -= 1
            mm = re.match(reg, val[b])
            if mm:
                directive = mm.group(1)
                return make_response(jsonify(getDirectiveOptionNames(directive, pre)), 200)

        # Check if its body content
        b = row
        ws = len(val[b])-len(val[b].lstrip())

        if ws > 0:
           while b > 0 :
               if len(val[b].lstrip()) == 0:
                   b-=1
               else:
                   wss = len(val[b]) - len(val[b].lstrip())
                   if wss == 0:

                       break
                   elif wss < ws:
                      ws = wss
                   b-=1

           mm = re.match(reg, val[b])
           if mm:
                a = row + 1
                while (a < len(val) and (len(val[a]) - len(val[a].lstrip()) == ws)):
                    a+=1
                directive = mm.group(1)
                return make_response(jsonify(getDirectiveBody(directive, val, row, col, b, a )), 200)

        if col > (len(pre) + 1) :
            v = val[row][0:col]
            if v[-1 * len(pre) - 1] == ":":
                return make_response(jsonify(getAllRoles()),200)

        if col == 0:
            return make_response(jsonify(getDirectiveList(".. ")),200)
        elif col == 1 and val[row][0] == "." :
            return make_response(jsonify(getDirectiveList(". ")),200)
        elif col == 2 and val[row][0:2] == ".." :
            return make_response(jsonify(getDirectiveList(" ")),200)
        elif col > 2 and val[row][0:3] == ".. ":
            return make_response(jsonify(getDirectiveList("")),200)
        else :
            return make_response(jsonify(getRoleList(col)),200)

    except Exception as e:
        print(e)
        pass

    return make_response(jsonify([]), 201)


@blueprint.route("/rst/raw/<int:id_>/<int:dataId>")
def rstraw(id_, dataId):
    try:

        with VnVFile.VnVFile.find(id_) as file:
            data = file.getById(dataId).cast()
            content = file.templates.get_raw_rst(data)
            return make_response(content, 200)
    except Exception as e:
        return make_response("Error\n-----", 200)

@blueprint.route('/workflow_node/<int:id_>')
def workflowNode(id_):
    try:
        
        with VnVFile.VnVFile.find(id_) as file:
            creator = request.args.get("creator")
            if creator is not None:
                return make_response(creator,200)
    except:
        pass
    return make_response("", 200)


@blueprint.route('/ip/<int:id_>')
def ip(id_):

    try:
        with VnVFile.VnVFile.find(id_) as file:
            injection = request.args.get('ipid', type=int)

            if injection == -100:
                return render_template("viewers/introduction.html",introRender=file.get_introduction())


            iprender = file.render_ip(injection)
            if iprender is None:
                return render_template("viewers/introduction.html", introRender=file.get_introduction())

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
        if injection < 0:
            return "<h4>Main Application</h4>"

        with VnVFile.VnVFile.find(id_) as file:

            iprender = file.render_ip(injection)
            return render_template("general/label.html", iprender=iprender)
    except Exception as e:
        return make_response("error", 200)


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
    except Exception as e:
        return make_response(jsonify("error"), 200)


@blueprint.route("next")
def next():
    id_ = request.args.get("id_", type=int)
    count = request.args.get("count", 100, type=int)
    try:
        with VnVFile.VnVFile.find(id_) as file:
            data = file.proc_iter_next(count)
            endtime = file.root.getEndTime()
            return jsonify({"data": data, "endtime": endtime}), 200
    except BaseException as e:
        return render_error(501, "Error Loading File")
