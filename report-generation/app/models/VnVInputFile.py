import json
import os
import textwrap
import uuid

import flask
import jsonschema
from ansi2html import Ansi2HTMLConverter
from flask import jsonify
from jsonschema.exceptions import ErrorTree, ValidationError, SchemaError

from app.base.utils import mongo
from app.models import VnV
from app.models.VnVConnection import VnVLocalConnection, VnVConnection, connectionFromJson
from app.models.json_heal import autocomplete
from app.rendering import render_rst_to_string
from app.rendering.vnvdatavis.directives.dataclass import DataClass
from app.rendering.vnvdatavis.directives.psip import GET_DEFAULT_PSIP


def get_current_path(newVal, row, col):
    i = 0
    while i < len(newVal):
        c = newVal[i]
        if c in map:
            looking = map[c]


def get_row_and_column(path, newVal, a):
    try:
        if len(path) == 0:
            return 1, 1
        p = a
        while len(path) > 1:
            p = p[path.popleft()]
        p[path.pop()] = "91123212"
        aa = json.dumps(a, separators=(',', ':')).find("91123212")

        s = newVal.split("\n")
        newlines = 0
        currcol = 0
        inquotes = False
        for i in newVal:
            sub = True
            currcol += 1
            if i == "\n":  # new line so reset col count and currcol
                sub = False
                newlines += 1
                currcol = 0
            elif i == " " and not inquotes:
                sub = False
            elif i == "\"" and (i == 0 or newVal[i - 1] != "\\"):
                inquotes = not inquotes
            if sub: aa -= 1
            if aa == 0: return newlines, currcol;
    except:
        pass

    return 1, 1


class Dependency:
    def __init__(self, remoteName=None, type=None, describe="", **kwargs):
        self.id_ = uuid.uuid4().hex
        self.remoteName = remoteName
        self.type = type
        self.desc = describe
        self.kwargs = kwargs

    def to_json(self):
        return {"id": self.id_, "type": self.type, "remoteName": self.remoteName, "desc": self.desc,
                "kwargs": self.kwargs}

    def describe(self):
        if len(self.desc) > 0:
            return self.desc

        if self.type in ["moose", "text"]:
            return "A " + self.type + " based input file (" + self.kwargs.get("text")[0:50] + "...)"
        elif self.type == ["upload"]:
            return f" A File uploaded from your local machine (" + self.kwargs.get("original", "None") + ")"
        else:
            return "A " + self.type + " of remote file \"" + self.kwargs.get("text", "") + "\""

    @staticmethod
    def from_json(j):
        d = Dependency()
        d.id_ = j["id"]
        d.remoteName = j["remoteName"]
        d.type = j["type"]
        d.desc = j.get("desc", "")
        d.kwargs = j["kwargs"]
        return d


class VnVInputFile:
    COUNTER = 0

    FILES = {}

    DEFAULT_SPEC = {}

    def __init__(self, name, path=None, **defs):
        self.name = name
        self.displayName = name
        self.filename = path if path is not None else "path/to/application"

        self.icon = "icon-box"
        self.id_ = VnVInputFile.get_id()
        self.connection = VnVLocalConnection()

        self.loadfile = ""
        self.additionalPlugins = {}

        self.spec = "{}"
        self.specLoad = {}

        self.execFile = ""
        self.specValid = False
        self.desc = None
        self.rendered = None

        #Set the command used to dump the specification
        self.specDump = "${application} ${inputfile}"
        if "specDump" in defs:
            self.specDump = defs["specDump"]

        #Set the execution file
        self.exec = json.dumps(self.defaultExecution, indent=4)
        self.exec = json.dumps(self.defaultExecution, indent=4)
        if "exec" in defs:
            self.exec = json.dumps(defs["exec"], indent=4)

        #Set the PSIP configuration if it exists.
        self.psip = "{}"
        self.psip_enabled = defs.get("psip_enabled", True)
        if "psip" in defs:
            self.psip = json.dumps(defs["psip"])


        #Add the issues if we have them
        self.issues = "[]"
        self.issues_enabled = defs.get("issues_enabled", True)

        if "issues" in defs:
            self.issues = json.dumps(defs["issues"])

        #Add all the dependencies. If it is an upload depdendency then
        # we have to special case it.
        self.deps = {}
        if "deps" in defs:
            for k,v in defs["deps"].items():
                self.add_dependency(**v)

        # Set the default Input file values.
        if "vnv_input" in defs:
            self.value = json.dumps(defs["vnv_input"], indent=4)
        else:
            self.value = json.dumps(VnV.getVnVConfigFile_1(), indent=4)

        # Update my plugins -- based on the input file.
        self.plugs = self.getPlugins()

        # Update my specification -- based on the input file.
        self.updateSpec()

    def toJson(self):
        a = {}
        a["name"] = self.name
        a["displayName"] = self.displayName
        a["filename"] = self.filename
        a["icon"] = self.icon
        a["connection"] = self.connection.toJson()
        a["loadfile"] = self.loadfile
        a["value"] = self.value
        a["spec"] = self.spec
        a["specDump"] = self.specDump
        a["psip"] = self.psip
        a["psip_enabled"] = self.psip_enabled
        a["issues"] = self.issues
        a["issues_enabled"] = self.issues_enabled
        a["deps"] = self.dump_dependencies()
        a["specValid"] = self.specValid
        a["exec"] = self.exec
        a["execFile"] = self.execFile

        a["rendered"] = self.rendered
        return a

    @staticmethod
    def fromJson(a):
        r = VnVInputFile(a["name"])
        r.filename = a["filename"]
        r.icon = a["icon"]
        r.displayName = a.get("displayName", r.name)  # backwards compat
        r.connection = connectionFromJson(a["connection"])
        r.load_dependencies(a.get("deps", "{}"))
        r.loadfile = a["loadfile"]
        r.value = a["value"]
        r.psip = a.get("psip", "{}")
        r.psip_enabled = a.get("psip_enabled",True)
        r.issues = a.get("issues", "[]")
        r.issues_enabled = a.get("issues_enabled", True)
        r.spec = a["spec"]
        r.specDump = a["specDump"]
        r.specValid = a["specValid"]
        r.rendered = a["rendered"]
        r.exec = a["exec"]
        r.execFile = a["execFile"]

        try:
            r.specLoad = json.loads(r.spec)
        except:
            r.specLoad = VnVInputFile.DEFAULT_SPEC.copy()

        r.plugs = r.getPlugins()

        return r

    def setConnection(self, hostname, username, password, port):
        if isinstance(self.connection, VnVConnection):
            self.connection.connect(username, hostname, port, password)
        else:
            self.connection = VnVConnection()
            self.connection.connect(username, hostname, int(port), password)

    def setConnectionLocal(self):
        if not isinstance(self.connection, VnVLocalConnection):
            self.connection = VnVLocalConnection()
        self.connection.connect("", "", "", "")

    def setFilename(self, fname, specDump):
        self.filename = fname
        self.specDump = specDump
        self.updateSpec()

    def getFileStatus(self):
        if self.connection.connected():
            if self.connection.exists(self.filename):
                if (self.specValid):
                    return ["green", "Valid"]
                else:
                    return ["blue", "Could not extract schema. Is this the path to a valid VnV executable?"]
            else:
                return ["#d54287", "Application does not exist"]
        else:
            return ["red", "Connection is not open"]

    def getPlugins(self, val=None):
        if val is None:
            val = self.value
        try:
            # if the input is valid, then do it the easy way.
            a = json.loads(val)
            return a.get("additionalPlugins", {})
        except:
            return {}

    # When user clicks save we save the input and update the plugins.
    # if auto is on, we should also update the specification.
    def saveInput(self, newValue):

        # If the plugins have changed then we need to update the spec.
        newPlugs = self.getPlugins(newValue)
        if (newPlugs != self.get_current_plugins()):
            self.plugs = newPlugs
            self.updateSpec()

        # update the value
        self.value = newValue

    def get_current_plugins(self):
        if self.plugs is None:
            self.plugs = self.getPlugins()
        return self.plugs

    def validateInput(self, newVal):
        try:
            a = json.loads(newVal)
            jsonschema.validate(a, schema=self.specLoad)
            return []
        except ValidationError as v:
            r, c = get_row_and_column(v.path, newVal, a)
            return [{"row": r, "column": c, "text": v.message, "type": 'warning', "source": 'vnv'}]
        except Exception as e:
            return [{"row": 1, "column": 1, "text": str(e), "type": 'warning', "source": 'vnv'}]



    def schema(self):
        return self.spec

    def load_dependencies(self, deps):
        self.deps = {k: Dependency.from_json(v) for k, v in json.loads(deps)}

    def dump_dependencies(self):
        return json.dumps({k: v.to_json() for k, v in self.deps.items()})

    def dependencies(self):
        return self.deps

    def add_dependency(self, remoteName, type, **kwargs):
        newD = Dependency(remoteName=remoteName, type=type, **kwargs)
        self.deps[newD.id_] = newD
        return self.deps

    def delete_dependency(self, id_):
        self.deps.pop(id_)
        return self.deps

    def edit_dependency(self, id_, remoteName, type, **kwargs):
        dep = self.deps.get(id_)
        if dep is not None:
            dep.remoteName = remoteName
            dep.type = type
            dep.kwargs = kwargs
        return self.deps

    def view_dependency(self, id_):
        return "Hmmmm"

    def updateSpec(self):
        self.specValid = False
        self.rendered = None
        self.specLoad = {"error": "No specification available"}
        self.spec = json.dumps(self.specLoad)

        if not self.connection.connected():
            return

        def getSpecDumpCommand(inputfilename):
            main = self.specDump.replace("${application}", self.filename)
            main = main.replace("${inputfile}", inputfilename)
            return main

        try:
            s = {"additionalPlugins": self.get_current_plugins()}
            s["schema"] = {"dump": True, "quit": True}
            path = self.connection.write(json.dumps(s), None)
            a = getSpecDumpCommand(path)

            res = self.connection.execute(a, env={"VNV_INPUT_FILE":path})
            a = res.find("===START SCHEMA DUMP===") + len("===START SCHEMA DUMP===")
            b = res.find("===END SCHEMA_DUMP===")
            if a > 0 and b > 0 and b > a:
                self.spec = res[a:b]
                self.specLoad = json.loads(self.spec)
                self.specValid = True
                self.rendered = self.get_executable_description()

        except Exception as e:
            pass

    NO_INFO = "No Application Information Available\n===================================="

    def get_executable_description_(self):
        if self.specLoad is not None:
            desc = self.specLoad.get("definitions", {}).get("executable", {})
            return desc.get("template", self.NO_INFO) if desc is not None else self.NO_INFO

        else:
            return self.NO_INFO

    VNVINPUTFILEDEF = 1022334234443

    def get_executable_description(self):
        if self.rendered is None:
            self.rendered = flask.render_template_string(render_rst_to_string(self.get_executable_description_()),
                                                         data=DataClass(self, self.id_, 1022334234443))
        return self.rendered

    def getId(self):
        return self.id_

    def validateSpec(self, newVal):
        try:
            s = json.loads(newVal)
            jsonschema.Draft7Validator.check_schema(s)
            return []
        except SchemaError as v:
            r, c = get_row_and_column(v.path, newVal, s)
            return [{"row": r, "column": c, "text": v.message, "type": 'warning', "source": 'vnv'}]
        except Exception as e:
            return [{"row": 1, "column": 1, "text": str(e), "type": 'warning', "source": 'vnv'}]

    def describe(self):
        return f'{self.connection.describe()}://{self.filename}'

    def execTemplate(self):
        return self.exec

    # TODO Implement this.
    CONFIG_SCHEMA = {
        "type": "object",
        "properties": {
            "name": {"type": "string"},
            "shell": {"type": "string", "enum": ["bash"]},
            "shebang": {"type": "string"},
            "working-directory": {"type": "string"},
            "load-vnv": {"type": "boolean"},
            "environment": {
                "type": "object"
            },
            "input-file-name": {"type": "string"},
            "command-line": {"type": "string",
                             "description": "Command to submit with input file called '${inputfilename}'"},
            "input-staging": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "action": {"type": "string", "enum": ["cp", "mv", "slink", "hlink"]},
                        "source": {"type": "string"},
                        "dest": {"type": "string"}
                    },
                    "required": ["action", "source", "dest"]
                }
            },
            "output-staging": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "action": {"type": "string", "enum": ["cp", "cp -r", "mv", "ln", "ln -s"]},
                        "source": {"type": "string"},
                        "dest": {"type": "string"}
                    },
                    "required": ["action", "source", "dest"]
                }
            }

        },
        "additionalProperties": False
    }

    EXECUTION_SCHEMA = None

    VNV_PREFIX = ""

    @staticmethod
    def getExecutionSchema():
        if VnVInputFile.EXECUTION_SCHEMA is None:
            VnVInputFile.EXECUTION_SCHEMA = json.loads(json.dumps(VnVInputFile.CONFIG_SCHEMA))
            VnVInputFile.EXECUTION_SCHEMA["properties"]["active_overrides"] = {"type": "array",
                                                                               "items": {"type": "string"}}
            VnVInputFile.EXECUTION_SCHEMA["properties"]["overrides"] = {
                "type": "object",
                "additionalProperties": json.loads(json.dumps(VnVInputFile.CONFIG_SCHEMA))
            }

        return VnVInputFile.EXECUTION_SCHEMA

    defaultExecution = {
        "shell": "bash",
        "load-vnv": True,
        "working-directory": "${application_dir}",
        "environment": {},
        "input-file-name": "./vv-input.json",
        "input-staging": [],
        "output-staging": [],
        "active_overrides": ["run"],
        "overrides": {
            "run": {
                "command-line": "${application} ${inputfile}",
                "name": "Hello"
            }
        }
    }

    def validateExecution(self, newVal):
        try:
            a = json.loads(newVal)
            print(a)
            errs = jsonschema.validate(a, schema=VnVInputFile.getExecutionSchema())
            return []
        except ValidationError as v:
            r, c = get_row_and_column(v.path, newVal, a)
            return [{"row": r, "column": c, "text": v.message, "type": 'warning', "source": 'vnv'}]
        except Exception as e:
            return [{"row": 1, "column": 1, "text": str(e), "type": 'warning', "source": 'vnv'}]

    def autocomplete_input(self, row, col, pre, val, plugins=None):
        return autocomplete(val, self.specLoad, int(row), int(col), plugins=plugins)

    def autocomplete_exec(self, row, col, pre, val, plugins=None):
        return autocomplete(val, VnVInputFile.getExecutionSchema(), int(row), int(col))

    def autocomplete_spec(self, row, col, pre, val, plugins=None):
        return []

    #MOOSE VALIDATION AND AUTOCOMPLETE
    #TODO Map these to a moose autocomplete and validation function.
    def autocomplete_moose(self,dep, dump, row, col, pre, val):
        spec = self.getSpec(dep,dump)
        #return autocomplete_hive(spec, val, row,col,pre)
        return [{"caption": "TODO", "value": "TODO", "meta": "", "desc": "HIVE (moose) Autocomplete is under active development"}]

    def getSpec(self,depId, dump):
        return None #TODO

    def validate_moose(self, depId, dump, newVal):
        dep = self.deps.get(depId)
        spec = self.getSpec(depId, dump)
        #return validate_hive(spec, newVal);
        return [{"row": 1, "column": 1, "text": "Validation is a work in progress", "type": 'warning', "source": 'vnv'}]

    def get_jobs(self):
        return [a for a in self.connection.get_jobs()]

    def fullInputFile(self):
        j = json.loads(self.value)
        if "actions" not in j:
            j["actions"] = {}

        if self.psip_enabled:
            j["actions"]["VNV:PSIP"] = json.loads(self.psip)

        if self.issues_enabled:
            j["actions"]["VNV:issues"] = json.loads(self.issues)

        return json.dumps(j, indent=4)

    def execute(self, val):
        inp_dir = json.loads(self.value).get("job", {}).get("dir", "/tmp")

        workflow_id = uuid.uuid4().hex
        script, name = self.script(val, workflow_id)

        meta = {
            "vnv_input": Ansi2HTMLConverter().convert(self.fullInputFile()),
            "workflow_id": workflow_id,
            "workflow_dir": inp_dir
        }

        return self.connection.execute_script(script, name=name, metadata=meta)

    def get_psip(self):
        if "sa" in self.psip and "ptc" in self.psip:
            return self.psip
        else:
            return GET_DEFAULT_PSIP()

    def get_issues(self):
        return self.issues  # .replace('\n', '\\\\n')

    def write_deps(self, workdir):
        #This script is called inside the working directory.
        s = []
        for k, dep in self.deps.items():
            if dep.type in ["text", "moose"]:
                s.append(textwrap.dedent(f"""
              cat << VVVEOF > {dep.remoteName}  
              {dep.kwargs.text}
              VVVEOF
              """))
            elif dep.type == "copy":
                s.append(f"cp {dep.kwargs.text} {dep.remoteName}")
            elif dep.type == "soft link":
                s.append(f"ln -s {dep.kwargs.text} {dep.remoteName}")
            elif dep.type == "hard link":
                s.append(f"ln {dep.kwargs.text} {dep.remoteName}")
            elif dep.type == "upload":
                self.connection.upload(os.path.join(workdir,dep.remoteName), dep.kwargs.text)

        return "\n".join(s)

    def script(self, val, workflowId):
        data = json.loads(val)
        for i in data.get("active_overrides", []):
            if i in data.get("overrides", {}):
                over = data["overrides"][i]
                for k, v in over.items():
                    data[k] = v

        # Deps Script is called from inside the working directory.
        deps = self.write_deps(data.get("working-directory", "${application_dir}"))
        
        return bash_script(self.filename, self.fullInputFile(), data, workflowName=workflowId, deps=deps), data.get("name")

    @staticmethod
    def get_id():
        VnVInputFile.COUNTER += 1
        return VnVInputFile.COUNTER

    @staticmethod
    def add(name, path=None, defs={}):

        a = mongo.loadInputFile(name)
        if a is not None:
            raise Exception("Name is taken")
        else:
            f = VnVInputFile(name, path=path, defs=defs )

        VnVInputFile.FILES[f.id_] = f
        return f

    @staticmethod
    def load(name):
        a = mongo.loadInputFile(name)
        if a is not None:
            f = VnVInputFile.fromJson(a)
            VnVInputFile.FILES[f.id_] = f
            return f

    @staticmethod
    def loadAll():
        for a in mongo.list_input_files():
            f = VnVInputFile.fromJson(a)
            VnVInputFile.FILES[f.id_] = f

    @staticmethod
    def removeById(fileId):
        a = VnVInputFile.FILES.pop(fileId)
        mongo.deleteInputFile(a.name)

    @staticmethod
    def delete_all():
        for k, v in VnVInputFile.FILES.items():
            mongo.deleteInputFile(v.name)
        VnVInputFile.FILES.clear()

    @staticmethod
    def find(id_):
        if id_ in VnVInputFile.FILES:
            return VnVInputFile.FileLockWrapper(VnVInputFile.FILES[id_])
        raise FileNotFoundError

    class FileLockWrapper:
        def __init__(self, file):
            self.file = file

        def __enter__(self):
            return self.file

        def __exit__(self, type, value, traceback):
            mongo.persistInputFile(self.file)


def njoin(array):
    return "\n".join(array)



def bash_script(application_path, inputfile, data, workflowName, deps):
    return textwrap.dedent(f"""
{data.get("shebang", "#!/usr/bin/bash")}

export application={application_path}
export application_dir=$(dirname {application_path})
export inputfile={data.get("input-file-name", ".vv-input.json")}

export VNV_WORKFLOW_ID={workflowName}

cd {data.get("working-directory", "${application_dir}")}

{deps}

cat << EOF > ${{inputfile}}
    {inputfile}
EOF

{njoin(["export " + k + "=" + v for k, v in data.get("environment", {})])}
{njoin([a["action"] + " " + a["source"] + " " + a["dest"]] for a in data.get("input-staging", {}))}
{data.get("command-line", "")}
{njoin([a["action"] + " " + a["source"] + " " + a["dest"]] for a in data.get("output-staging", {}))} 
""")
