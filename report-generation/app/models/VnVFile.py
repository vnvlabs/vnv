import json
import os
import random
import shutil
import threading
import time
from datetime import datetime
from urllib.request import pathname2url

import jsonschema
import pygments.formatters.html
from pygments.lexers import guess_lexer, guess_lexer_for_filename

from app.base.utils import mongo
from app.base.utils.mongo import validate_name, Configured
from app.models.VnVConnection import VnVConnection, VnVLocalConnection
from python_api.VnVReader import node_type_START, node_type_POINT, node_type_DONE, node_type_ITER, node_type_ROOT, \
    node_type_LOG, \
    node_type_END, node_type_WAITING
from flask import render_template, make_response, render_template_string

from app.models import VnV
import app.rendering as r
from app.rendering.readers import has_reader, LocalFile
from app.rendering.vnvdatavis.directives.dataclass import render_vnv_template, DataClass


class ProvFileWrapper:

    def __init__(self, vnvfileid, pfile, description):
        self.file = pfile
        self.vnvfileid = vnvfileid
        self.description = description

    def getVnVFileId(self):
        return self.vnvfileid

    def getName(self):
        return self.file.info.name

    def getUrl(self):
        return pathname2url(self.getName())

    def getSize(self):
        return self.file.info.size

    def getTimeStamp(self):
        return self.file.info.timestamp

    def modified(self):
        return self.file.modified()

    def reader(self):
        return self.file.reader

    def text(self):
        return self.file.text

    def package(self):
        return self.file.package

    def comm(self):
        return self.file.comm

    def readable(self):
        return has_reader(self.file.reader)

    def getDescription(self):
        return self.description


class ProvWrapper:

    def __init__(self, vnvfileid, vnvprov, templates):
        self.prov = vnvprov
        self.templates = templates
        self.vnvfileid = vnvfileid

    def getD(self, provfile):
        return render_template(
            self.templates.get_file_description(
                provfile.package, provfile.name))

    def get_executable(self):
        return ProvFileWrapper(self.vnvfileid, self.prov.executable, "")

    def getVnVFileId(self):
        return self.vnvfileid

    def get_command_line(self):
        return self.prov.commandLine

    def get_libraries(self):
        a = [ProvFileWrapper(self.vnvfileid, self.prov.get(a, 2), "") for a in range(0, self.prov.size(2))]
        a.sort(key=lambda x: x.modified(), reverse=True)
        return a

    def get_outputs(self):
        r = []
        for i in range(0, self.prov.size(1)):
            a = self.prov.get(i, 1)
            r.append(ProvFileWrapper(self.vnvfileid, a, self.getD(a)))
        return r

    def get_inputs(self):
        r = []
        for i in range(0, self.prov.size(0)):
            a = self.prov.get(i, 0)
            r.append(ProvFileWrapper(self.vnvfileid, a, self.getD(a)))
        return r

    def has_inputs(self):
        return self.prov.size(0) > 0

    def has_outputs(self):
        return self.prov.size(1) > 0

    def get_input(self):
        return ProvFileWrapper(self.vnvfileid, self.prov.inputFile, "")

    def get_timestamp(self):
        t = self.prov.time_in_seconds_since_epoch
        return datetime.fromtimestamp(t).strftime('%Y-%m-%d %H:%M:%S')

    def render_input_file(self):
        lex = pygments.lexers.get_lexer_by_name("json")
        form = pygments.formatters.html.HtmlFormatter(
            linenos=True, style="colorful", noclasses=True)
        return pygments.highlight(self.get_input().text(), lex, form)


class CommObj:

    def __init__(self, comm_map, node_map, world_size):
        self.comm_map = comm_map
        self.node_map = node_map
        self.world_size = world_size

    def keys(self):
        return self.comm_map.keys()


class CommRender:
    def __init__(self, comm, commObj):
        self.comm = comm
        self.commObj = commObj
        self.html = "TODO"

    def getData(self):
        res = {}
        m = self.getMatSize()
        for node, procs in self.commObj.node_map.items():
            for proc in procs:
                x, y = self.getIndex(proc, m)
                if y in res:
                    res[y]["data"].append({"x": x, "y": int(
                        node) * (1 if self.inComm(proc) else -1), "r": proc, "o": self.inComm(proc)})
                else:
                    res[y] = {"name": f"Row {y}", "row": y, "data": [{"x": x, "y": int(
                        node) * (1 if self.inComm(proc) else -1), "r": proc, "o": self.inComm(proc)}]}
        print(res)
        return json.dumps(sorted(res.values(), key=lambda x: x["row"]))

    def inComm(self, proc):
        return proc in self.commObj.comm_map[self.comm]

    def getIndex(self, rank, matSize):
        return int(rank / matSize), rank % matSize

    def getMatSize(self):
        i = 1
        while self.commObj.world_size > i * i:
            i += 1
        return i


class LogRender:
    def __init__(self, data, commObj, templates):
        self.data = data
        self.commObj = commObj
        self.templates = templates

    def get_comm(self):
        return CommRender(self.data.getComm(), self.commObj)

    def getMessage(self):
        return self.data.getMessage()

    def getPackage(self):
        return self.data.getPackage()

    def getTime(self):
        return self.data.getTime()

    def getLevel(self):
        return self.data.getLevel()

    badges = {
        "DEBUG": "success",
        "INFO": "info",
        "WARN": "warning",
        "ERROR": "danger"
    }

    def getBadge(self):
        return self.badges.get(self.getLevel(), "secondary")

    def getStage(self):
        return self.data.getStage()


class UnitTestRender:

    def __init__(self, unitTest, commObj, templates):
        self.commObj = commObj
        self.data = unitTest
        self.templates = templates

    def getTests(self):
        return [a for a in self.data.getResults().fetchkeys()]

    def getResult(self, name):
        try:
            return self.data.getResults().get(name).getResult()
        except:
            return False

    def getRST(self, name):
        t = self.templates.get_unit_test_test_content(self.data.getPackage(), self.data.getName(), name);
        return render_vnv_template(t, self.data.getData(), file=self.templates.file)

    def getHtml(self):
        t = self.templates.get_html_file_name(
            "UnitTests", self.data.getPackage(), self.data.getName())
        return render_vnv_template(t, data=self.data.getData(), file=self.templates.file)


class RequestRender:

    def __init__(self, request, commObj, templates):
        self.request = request
        self.commObj = commObj
        self.templates = templates

    def getSchema(self):
        return json.loads(self.request.getSchema())

    def presentSchema(self):
        return json.dumps(self.getSchema(), indent=4)

    def getExpiry(self):
        return self.request.getExpiry()

    def getExpiryInSeconds(self):
        return self.getExpiry() - int(time.time())

    def getId(self):
        return self.request.getId()

    def getJID(self):
        return self.request.getJID()

    def getMessage(self):
        mess = self.request.getMessage()
        return mess


class TestRender:

    def __init__(self, data, commObj, templates, template_override=None):
        self.data = data
        self.commObj = commObj
        self.templates = templates
        self.template_override = template_override

    def getData(self):
        return self.data.getData()

    def getTitle(self):
        t = self.templates.get_title("Tests", self.data.getPackage(), self.data.getName(), short=False)
        if len(t) > 0:
            a = render_vnv_template(t, data=self.data.getData(), file=self.templates.file)
            # Remove any paragraph tags that shpinx added --
            return a.replace("<p>", "<span>").replace("</p>", "</span>")

        return self.data.getPackage() + " - " + self.data.getName()

    def getHtml(self):
        if self.template_override is None:
            t = self.templates.get_html_file_name(
                "Tests", self.data.getPackage(), self.data.getName())
            return render_vnv_template(t, data=self.data.getData(), file=self.templates.file)

        return render_vnv_template(
            self.template_override,
            data=self.data.getData(), file=self.templates.file)

    def getName(self):
        return self.data.getName()

    def getId(self):
        return self.data.getId()

    def getFile(self):
        return self.templates.file

    def getPackage(self):
        return self.data.getPackage()

    def getLogs(self):
        return [LogRender(a, self.commObj, self.templates)
                for a in self.data.getLogs()]


class PackageRender:
    def __init__(self, package, data, templates):
        self.data = data
        self.package = package
        self.templates = templates

    def getHtml(self):
        t = self.templates.get_package(self.package)
        a = render_vnv_template(t, data=self.data.getData(), file=self.templates.file)
        if len(a) > 0:
            return a
        else:
            return "<h4> No Package Information Available </h>"

    def getLogs(self):
        return [LogRender(a, self.commObj, self.templates)
                for a in self.data.getLogs()]


class IntroductionRender:

    def __init__(self, package, root, templates):
        self.root = root
        self.templates = templates
        self.package = package

    def getTitle(self, short=False):
        t = self.templates.get_intro_title(short=short)
        a = render_vnv_template(t, data=self.root, file=self.templates.file)
        if len(a) > 0:
            return a.replace("<p>", "<span>").replace("</p>", "</span>")
        return self.package

    def getFile(self):
        return self.templates.file

    def getId(self):
        return self.root.getId()

    def getHtml(self):
        return render_vnv_template(self.templates.get_introduction(), data=self.root, file=self.templates.file)

    def getRawRST(self):
        return self.templates.get_raw_introduction()

    def getRequest(self):
        return None


class WorkflowRender:

    def __init__(self, workflowNode, template_root, root, templates):
        self.workflowNode = workflowNode
        self.root = root
        self.name = workflowNode.getName()
        self.template_root = template_root
        self.templates = templates
        self.package = workflowNode.getPackage()
        self.creators = []
        self.codeNameMap = {}
        self.infoStr = '{"nodes":[],"links":[]}'
        self.reports = {}
        self.loadWorkflow()

    def getTitle(self):
        return self.package + ":" + self.name

    def getFile(self):
        return self.templates.file

    def getId(self):
        return self.workflowNode.getId()

    def getWorkflowGraph(self):
        self.loadWorkflow()
        return self.infoStr

    def getVnVFile(self, name, engineInfo):

        # if the job name matches then its that one
        f = VnVFile.findByJobName(name)
        if f is not None:
            return f

        # if the engine name matches and the reader matches then its prob that one.
        for k, v in VnVFile.FILES.items():
            if v.filename == engineInfo["filename"] and v.reader == engineInfo["reader"]:
                return v

        # make a new one.
        ff = VnVFile.add(name, engineInfo["filename"], engineInfo["reader"], self.template_root, False,
                         **engineInfo)
        return ff

    def loadWorkflow(self):
        if self.workflowNode.getInfoStr() != self.infoStr:
            self.infoStr = self.workflowNode.getInfoStr()
            s = json.loads(self.infoStr)
            creators = {}
            codeNameMap = {}

            for i in s["nodes"]:

                if i["type"] == "Job":
                    codeNameMap[i["value"]["code"]] = i["value"]["name"]
                    c = i["value"]["creator"]
                    cc = c.split(":")
                    jobH = self.render_job_html(cc[0], cc[1], i["value"]["name"])
                    if c not in creators:
                        ht = render_vnv_template(self.templates.get_job_creator(cc[0], cc[1]), data=self.workflowNode,
                                                 file=self.templates.file)
                        creators[c] = {"name": c, "html": ht, "jobs": [{"name": i["value"]["name"], "html": jobH}]}
                    else:
                        creators[c]["jobs"].append({"name": i["value"]["name"], "html": jobH})
                elif i["type"] == "VnVReport":
                    val = i["value"]
                    if not self.workflowNode.hasReport(val["alias"]):
                        ff = self.getVnVFile(val["name"], val["engine"])
                        self.reports[val["alias"]] = ff
                        self.workflowNode.setReport(val["alias"], ff.id_, ff.root)

            self.creators = [v for k, v in creators.items()]
            self.codeNameMap = codeNameMap

    def render_job_html(self, package, name, jobName):
        return render_vnv_template(self.templates.get_job_creator_job(package, name, jobName),
                                   data=self.workflowNode, file=self.templates.file)

    def getHtml(self, package, name, code=None):
        self.loadWorkflow()
        if code is not None:
            jobName = self.codeNameMap[code]
            return self.render_job_html(package, name, jobName)
        return render_vnv_template(self.templates.get_job_creator(package, name), data=self.workflowNode,
                                   file=self.templates.file)

    def getWorkflowCreators(self):
        self.loadWorkflow()
        return self.creators

    def getRawRST(self, package, name, jobName=None):
        if jobName is not None:
            return self.templates.get_raw_job_creator_job(package, name, jobName)
        return self.templates.get_raw_job_creator(package, name)

    def getRequest(self):
        return None


class ActionRender:
    def __init__(self, package, data, templates):
        self.data = data
        self.package = package
        self.templates = templates

    def getHtml(self):
        t = self.templates.get_action(self.data.getPackage(), self.data.getName())
        a = render_vnv_template(t, data=self.data.getData(), file=self.templates.file)
        if len(a) > 0:
            return a
        else:
            return "<h4> No Package Information Available </h>"

    def getLogs(self):
        return [LogRender(a, self.commObj, self.templates)
                for a in self.data.getLogs()]


class SourceFile:
    def __init__(self, file, line, vnvfileid):
        self.file = file
        self.line = line
        self.vnvfileid = vnvfileid

    def getFilename(self):
        return self.file

    def getLine(self):
        return self.line

    def hasLine(self):
        return self.line is not None

    def getLineList(self):
        return [self.getLine()] if self.hasLine() else []

    ### WANT TO RENDER THE SOURCE CODE USING THE BROWSER -
    ### MAKE THE BUTTONS CHANGE THE FILE.

    def render(self):
        try:
            with open(self.file, 'r') as f:
                d = f.read()
                lex = guess_lexer_for_filename(self.file, d, stripnl=False)
                form = pygments.formatters.html.HtmlFormatter(
                    linenos=True, hl_lines=self.getLineList(), style="colorful", cssclass="vnvhigh")
                result = pygments.highlight(d, lex, form)
        except Exception as e:
            result = "Could not load file with name " + self.file

        return result


class InjectionPointRender:

    def __init__(self, injection_point, templates, commObj):
        self.ip = injection_point.cast()
        self.templates = templates
        self.commObj = commObj

    def getId(self):
        return self.ip.getId()

    def getRawRST(self):
        return self.templates.get_raw_rst(self.ip)

    def getTitle(self, short=False):
        t = self.templates.get_title("InjectionPoints", self.ip.getPackage(), self.ip.getName(), short=short)
        a = render_vnv_template(t, data=self.ip.getData().getData(), file=self.templates.file)
        if len(a) > 0:
            return a.replace("<p>", "<span>").replace("</p>", "</span>")
        return  self.ip.getName()

    def getFile(self):
        return self.templates.file

    def getCommRender(self):
        return CommRender(self.ip.getComm(), self.commObj)

    def getName(self):
        return self.ip.getName()

    def getPackage(self):
        return self.ip.getPackage()

    def getStatus(self):
        if self.getRequest() is not None:
            return "Waiting"
        elif self.open():
            return "Processing"
        else:
            return "Complete"

    def open(self):
        return self.ip.getopen()

    def getInternalTest(self):
        tempoverride = self.templates.get_html_file_name(
            "InjectionPoints", self.ip.getPackage(), self.ip.getName())
        return TestRender(self.ip.getData(), self.commObj, self.templates,
                          template_override=tempoverride)

    def getRequest(self):
        r = self.ip.getData().getFetchRequest()
        if r is not None and r.getExpiry() - int(time.time()) > 5:
            return RequestRender(r, self.commObj, self.templates)

        for rr in self.ip.getTests():
            r = rr.getFetchRequest()
            if r is not None and r.getExpiry() - int(time.time()) > 5:
                return RequestRender(r, self.commObj, self.templates)
        return None

    def getAdditionalTests(self):
        return [TestRender(a.cast(), self.commObj, self.templates)
                for a in self.ip.getTests()]

    def getLogs(self):
        return [LogRender(a.cast(), self.commObj, self.templates)
                for a in self.ip.getLogs()]

    def getSourceMap(self):
        vnvfile = VnVFile.FILES[self.getFile()]
        conn = vnvfile.connection

        # map stageId -> [ filename, line]
        #source = json.loads(self.ip.getSourceMap())

        ss = self.templates.getSourceMap(self.getPackage(),self.getName())
        return { a: LocalFile(b["filename"],self.getFile(),conn) for a,b in ss.items()}

        #return {a: LocalFile(b[0], self.getFile(), conn, highlightline=b[1]) for a, b in source.items()}


class VnVFile:
    COUNTER = 0

    FILES = VnV.FILES

    def getReaderConfig(self, username, password):
        if not Configured():
            a = {"persist": "memory"}
        else:
            a = {
                "uri": "mongodb://localhost:27017",
                "db": "vnv",
                "collection": self.name,
                "persist": "mongo"
            }
        if username is not None and len(username) > 0 and password is not None and len(password) > 0:
            a["username"] = username
            a["password"] = password

        return a

    def __init__(self, name, filename, reader, template_root, icon="icon-box", _cid=None, reload=False,
                 **kwargs):

        self.filename = filename
        self.reader = reader
        self.icon = icon
        self.options = kwargs
        self.template_root = template_root
        self.id_ = VnVFile.get_id() if _cid is None else _cid
        self.notifications = []
        self.workflowRender = None

        if not reload:
            self.dispName = name
            self.name = validate_name(name)
            mongo.update_display_name(self.name, self.dispName)
        else:
            self.name = name
            self.dispName = mongo.get_display_name(self.name)

        self.wrapper = VnV.Read(filename, reader, self.getReaderConfig(self.options.get("username"),
                                                                       self.options.get("password")))

        self.root = self.wrapper.get()
        self.template_dir = os.path.join(template_root, str(self.id_))
        # By default we have a localhost connection.
        self.setConnectionLocal()

        shutil.rmtree(self.template_dir, ignore_errors=True)

        self.th = None
        self.templates = None
        # self.setupNow()

    # Try and setup the templates once we can.
    def setupNow(self):
        if self.templates is None and self.th is None:
            self.vnvspec = json.loads(self.root.getVnVSpec().get())
            if self.vnvspec is not None and len(self.vnvspec) > 0:
                self.th = threading.Thread(target=self.setup_thread)
                self.th.start()
        return self.templates is not None

    def nospec(self):
        return self.vnvspec is None or len(self.vnvspec) == 0

    def setup_thread(self):
        self.templates = r.build(self.template_dir, self.vnvspec, self.id_)
        self.vnvspec = None

    def update_dispName(self, newName):
        self.dispName = newName
        mongo.update_display_name(self.name, newName)

    def render_temp_string(self, content):
        if self.setupNow():
            return self.templates.render_temp_string(content)

    def render_to_string(self, content):
        if self.setupNow():
            return self.templates.render_to_string(content)

    def get_raw_rst(self, data):
        if self.setupNow():
            return self.templates.get_raw_rst(data)

    def setConnection(self, hostname, username, password, port):
        self.connection = VnVConnection()
        self.connection.connect(username, hostname, int(port), password)

    def setConnectionLocal(self):
        self.connection = VnVLocalConnection()

    def getWorkflow(self):
        if self.setupNow():
            return self.root.getInfoNode().getWorkflow()
        return None

    def render_workflow_job(self, package, name, code=None):
        return self.getWorkflowRender().getHtml(package, name, code)

    def render_workflow_rst(self, package, name, jobName=None):
        return self.getWorkflowRender().getRawRST(package, name, jobName)

    def getJobName(self):
        if self.setupNow():
            return self.root.getInfoNode().getJobName()
        return None

    def clone(self):
        return VnVFile(self.name, self.filename, self.reader, self.template_root, self.icon, _cid=self.id_)

    def getDataRoot(self):
        return self.getDataChildren("#")

    def getDataChildren(self, nodeId):
        if nodeId == "#":
            return [
                f"Filename: {self.filename}",
                f"Reader: {self.reader}",
                {
                    "text": self.root.getName(),
                    "li_attr": {
                        "fileId": self.id_,
                        "nodeId": self.root.getId()
                    },
                    "children": True
                }
            ]
        else:
            node = self.getById(int(nodeId)).cast()
            a = json.loads(node.getDataChildren(self.id_, 2))
            return a

    def getCommRender(self, id):
        return CommRender(id, self.getCommObj())

    def isProcessing(self):
        return self.root.processing();

    def getById(self, dataid):
        return self.root.findById(dataid,False)

    def query(self, dataid, query):
        data = self.getById(dataid)
        if data is not None:
            return DataClass(data, dataid, self.id_).query(query)

    def query_str(self, dataid, query):
        data = self.getById(dataid).cast()
        if data is not None:
            return DataClass(data, dataid, self.id_).query_str(query)

    def respond(self, ipid, id_, jid, response):
        try:

            iprender = self.render_ip(ipid)
            r = iprender.getRequest()
            if r is not None:
                resp = json.loads(response)
                schema = r.getSchema()
                jsonschema.validate(resp, schema)
                self.root.respond(id_, jid, response)
                return make_response("Success", 200)
        except Exception as e:
            pass

        return make_response("Failed", 202)

    def get_world_size(self):
        a = self.root.getCommInfoNode()

        return self.root.getCommInfoNode().getWorldSize()

    def getPackages(self):
        return [{"name": a} for a in self.root.getPackages().fetchkeys()]

    def getPackage(self, package):
        return self.root.getPackage(package)

    def get_cwd(self):
        return self.root.getInfoNode().getProv().currentWorkingDirectory

    def browse(self):
        return LocalFile(self.get_cwd(), self.id_, self.connection, reader="directory")

    def getFirstPackage(self):
        a = self.getPackages()
        if len(a) > 0:
            return self.render_package(a[0]["name"])
        return ""

    def displayName(self, a):
        return " ".join([i.capitalize() for i in a.replace("_", " ").split(":")[-1].split(" ")])

    def getActions(self):
        return [{"name": a, "id_": n, "display_name": self.displayName(a)} for n, a in
                enumerate(self.root.getActions().fetchkeys())]

    def getFirstAction(self):
        a = self.getActions()
        if len(a) > 0:
            return self.render_action(a[0]["name"])
        return ""

    def getGlobalCommMap(self):
        return self.root.getCommInfoNode().getCommMap().toJsonStr(False)

    def getWorkflowRender(self):

        if self.workflowRender is None:
            n = self.root.getWorkflowNode()
            self.workflowRender = WorkflowRender(n, self.template_root, self.root, self.templates)

        return self.workflowRender

    def hasComm(self):
        return self.root.getCommInfoNode().getWorldSize() > 1

    DEBUG_WORKFLOW = False

    def hasWorkflow(self):
        if self.root.getWorkflowNode() is not None:
            s = self.root.getWorkflowNode().getInfoStr()
            try:
                ss = json.loads(s)
                return VnVFile.DEBUG_WORKFLOW or len(ss.get("nodes", [])) > 1
            except:
                return False

    def renderDefaultComm(self):

        j = json.loads(self.getGlobalCommMap())
        if "nodes" in j and len(j["nodes"]) > 0 and "id" in j["nodes"][0]:
            i = j["nodes"][0]["id"]
            try:
                a = render_template(
                    "files/comm.html",
                    commrender=self.getCommRender(
                        str(i)))
                return a
            except Exception as e:
                print(e)
        return ""

    class RankInfo:

        def __init__(self, commInfoNode):
            self.size = commInfoNode.getWorldSize()

            nmap = json.loads(commInfoNode.getNodeMap())
            self.nodes = len(nmap)
            self.min_rpn = len(
                nmap[min(nmap.keys(), key=lambda x: len(nmap[x]))])
            self.max_rpn = len(
                nmap[max(nmap.keys(), key=lambda x: len(nmap[x]))])
            self.version = commInfoNode.getVersion()
            self.unique_comms = len(
                json.loads(
                    commInfoNode.getCommMap().getComms()))

        def shortversion(self, length):
            return self.version[0: min(len(self.version), length)] + "..."

    def getRankInfo(self):

        return VnVFile.RankInfo(self.root.getCommInfoNode())

    def getDefaultComm(self):
        j = json.loads(self.getGlobalCommMap())
        if "nodes" in j and len(j["nodes"]) > 0 and "id" in j["nodes"][0]:
            return j["nodes"][0]["id"]

    def list_unit_test_packages(self):
        return [{"name": a.getPackage() + ":" + a.getName()}
                for a in self.root.getUnitTests()]

    def hasUnitTests(self):
        return len(self.root.getUnitTests())

    def getLogs(self):
        return [LogRender(a, self.getCommObj(), self.templates) for a in self.root.getLogs()]

    def getLogFilters(self):

        a = set()
        for i in self.root.getLogs():
            a.add(i.getLevel())
        return a

    def getLogPackages(self):
        a = set()
        for i in self.root.getLogs():
            a.add(i.getPackage())
        return a

    def hasLogs(self):
        return len(self.root.getLogs())

    def unit_test_table(self):
        data = {}
        if (self.hasUnitTests()):
            for i in self.root.getUnitTests():
                dd = {"name": i.getName(), "description": "", "id": i.getId()}

                result = True;
                child = []
                for nn, iik in enumerate(i.getResults().fetchkeys()):
                    ii = i.getResults().get(iik)
                    if not ii.getResult():
                        result = False;
                    child.append({"name": ii.getName(), "description": ii.getDescription(), "result": ii.getResult(),
                                  "id": i.getId(), "sid": nn})
                dd["_children"] = child;
                dd["result"] = result

                if i.getPackage() not in data:
                    data[i.getPackage()] = {"name": i.getPackage(), "result": True, "description": "",
                                            "id": i.getPackage(), "_children": []}
                data[i.getPackage()]["_children"].append(dd)
                if not dd["result"]:
                    data[i.getPackage()]["result"] = False
        root = [
            {"name": "Unit Testing", "description": "", result: True, "_children": list(data.values()), "id": "root"}]
        for i in root[0]["_children"]:
            if not i["result"]:
                root[0]["result"] = False
                break

        return json.dumps(root)

    def render_unit_test(self, id=None, utest=None):

        try:
            for i in self.root.getUnitTests():
                if utest == i.getPackage():
                    r = []
                    for a in self.root.getUnitTests():
                        if utest == a.getPackage():
                            r.append(UnitTestRender(a, self.getCommObj(), self.templates))

                    return render_template("viewers/unittestpackage.html",
                                           unitrenders=r, package=utest)

                if i.getId() == id:
                    if utest == i.getName():
                        return render_template("viewers/unittest.html",
                                               unitrender=UnitTestRender(i, self.getCommObj(), self.templates))
                    else:
                        return render_template("viewers/unittest.html",
                                               unitrender=UnitTestRender(i, self.getCommObj(), self.templates),
                                               utest=utest)

        except Exception as e:
            pass
        return "<div> no information available </div>"

    def get_comm_map(self):
        x = self.root.getCommInfoNode().getCommMap()
        s = x.getComms()
        return json.loads(s)

    def get_prov(self):
        return ProvWrapper(self.id_, self.root.getInfoNode().getProv(), self.templates)

    def get_node_map(self):
        x = self.root.getCommInfoNode().getNodeMap()
        return json.loads(x)

    def getCommObj(self):
        if not hasattr(self, "commObj"):
            self.commObj = CommObj(
                self.get_comm_map(),
                self.get_node_map(),
                self.get_world_size())
        return self.commObj

    def list_communicators(self):
        return self.getCommObj().keys()

    def get_injection_point(self, id):
        if (id == 226 ) :
            print ("GGGGGGGGGGGGGGGGGG")
        a = self.getById(id)
        if a is not None and a.getTypeStr() == "InjectionPoint":
            return a.cast()
        return None


    def list_injection_points(self, proc):
        """Return a nested heirarchy of injection points and logs for this comm."""
        return self.root.single_proc_heirarchy(proc)

    def get_introduction(self):
        return IntroductionRender("VnV Application", self.root, self.templates)

    def get_conclusion(self):
        return render_template(self.templates.get_conclusion())

    def render_ip(self, id):
        if self.templates is None:
            return None
        if id == VnVFile.INJECTION_INTRO:
            return self.get_introduction()
        elif id == VnVFile.INJECTION_CONC:
            return self.get_conclusion()
        ip = self.get_injection_point(id)
        if ip is not None:
            return InjectionPointRender(ip, self.templates, self.getCommObj())

        return None


    def render_package(self, package):
        packageTestObject = self.root.getPackage(package)
        return PackageRender(
            package,
            packageTestObject,
            self.templates).getHtml()

    def render_action(self, package):
        packageTestObject = self.root.getAction(package)
        return ActionRender(
            package,
            packageTestObject,
            self.templates).getHtml()

    def reset_proc_iter(self):
        self.proc_iter = None

    def set_proc_iter(self, proc=0, only=None):
        self.proc_iter_config = {
            "id": int(proc),
            "only": True if only else False,
            "comm": False}
        self.proc_iter = self.root.getWalker(
            "VNV", "proc", json.dumps(
                self.proc_iter_config))
        self.currX = -1
        self.currY = -1
        self.parents = []
        return True

    def set_comm_iter(self, comm, only=None):
        self.proc_iter_config = {
            "id": int(comm),
            "only": True if only else False,
            "comm": True}
        self.proc_iter = self.root.getWalker("VNV", "proc", json.dumps(self.proc_iter_config))
        self.parents = None
        return True

    node_type_map = {
        node_type_POINT: "point",
        node_type_START: "start",
        node_type_END: "end"
    }
    INJECTION_INTRO = -100
    INJECTION_CONC = -101

    def waiting(self, id_):
        r = self.render_ip(id_)
        if r is not None:
            return r.getRequest() is not None
        return False

    def proc_iter_next(self, count=None):

        res = []
        if self.currX == -1:
            res.append(
                {
                    "id": VnVFile.INJECTION_INTRO,
                    "title": self.getTitle(None, short=True),
                    "type": "start",
                    "package": "Root"
                }
            )

        while True:

            n = self.proc_iter.next()
            if n is not None:

                if n.type == node_type_DONE:
                    res.append({

                        "id": VnVFile.INJECTION_INTRO,
                        "type": "end",
                        "title": self.getTitle(None, short=True),
                        "package": "VnV",
                        "done": True
                    })
                    break

                elif n.type == node_type_WAITING:
                    break

                elif n.type in [node_type_POINT, node_type_END, node_type_START]:
                    ip = self.get_injection_point(n.item.getId())
                    if ip is not None:
                      res.append({
                        "title": self.getTitle(ip, short=True),
                        "id": n.item.getId(),
                        "package": ip.getPackage(),
                        "type": VnVFile.node_type_map[n.type]})

        return res

    def getTitle(self, ip, short=False):
        if ip is not None:
            return InjectionPointRender(ip, self.templates, self.getCommObj()).getTitle(short=short)
        return self.get_introduction().getTitle()

    def lock(self):
        self.root.lock()

    def release(self):
        self.root.release()

    @staticmethod
    def get_id():
        VnVFile.COUNTER += 1
        return VnVFile.COUNTER

    @staticmethod
    def add(name, filename, reader, template_root, reload=False, **kwargs):

        f = VnVFile(name, filename, reader, template_root, reload=reload, **kwargs)
        VnVFile.FILES[f.id_] = f
        return f

    @staticmethod
    def findByJobName(jname):
        for k, v in VnVFile.FILES.items():
            if v.getJobName() == jname:
                return v
        return None

    @staticmethod
    def removeById(fileId, refresh):
        p = VnVFile.FILES.pop(fileId)
        mongo.removeFile(p.name)
        if refresh:
            VnVFile.FILES[p.id_] = p.clone()

    @staticmethod
    def delete_all():

        for k, v in VnVFile.FILES.items():
            mongo.removeFile(v.name)
        VnVFile.FILES.clear()

    class FileLockWrapper:
        def __init__(self, file):
            self.file = file

        def __enter__(self):
            self.file.lock()
            return self.file

        def __exit__(self, type, value, traceback):
            self.file.release()

    @staticmethod
    def find(id_):
        if id_ in VnVFile.FILES:
            return VnVFile.FileLockWrapper(VnVFile.FILES[id_])
        raise FileNotFoundError
