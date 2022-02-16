import json
import os
import textwrap
import uuid

import jsonschema
from jsonschema.exceptions import ErrorTree, ValidationError, SchemaError

from app.base.utils import mongo
from app.models import VnV
from app.models.VnVConnection import VnVLocalConnection, VnVConnection, connectionFromJson
from app.models.json_heal import autocomplete


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


class VnVInputFile:
    COUNTER = 0

    FILES = {}

    DEFAULT_SPEC = {}

    def __init__(self, name, path=None):
        self.name = name
        self.filename = path if path is not None else "path/to/application"
        self.icon = "icon-box"
        self.id_ = VnVInputFile.get_id()
        self.notifications = []
        self.connection = VnVLocalConnection()

        self.loadfile = ""
        self.additionalPlugins = {}
        self.spec = "{}"
        self.specLoad = {}
        self.specDump = "${application} ${inputfile}"
        self.specAuto = True
        self.exec = json.dumps(self.defaultExecution, indent=4)
        self.execFile = ""

        self.plugs = None

        # Set the default Input file.
        self.saveInput(json.dumps(VnV.getVnVConfigFile_1(), indent=4))

    def toJson(self):
        a = {}
        a["name"] = self.name
        a["filename"] = self.filename
        a["icon"] = self.icon
        a["connection"] = self.connection.toJson()
        a["loadfile"] = self.loadfile
        a["value"] = self.value
        a["spec"] = self.spec
        a["specAuto"] = self.specAuto
        a["specDump"] = self.specDump
        a["exec"] = self.exec
        a["execFile"] = self.execFile
        return a

    @staticmethod
    def fromJson(a):
        r = VnVInputFile(a["name"])
        r.filename = a["filename"]
        r.icon = a["icon"]

        r.connection = connectionFromJson(a["connection"])

        r.loadfile = a["loadfile"]
        r.value = a["value"]

        r.spec = a["spec"]
        r.specAuto = a.get("specAuto", True)
        r.specDump = a["specDump"]
        r.exec = a["exec"]
        r.execFile = a["execFile"]

        try:
            r.specLoad = json.loads(r.spec)
        except:
            r.specLoad = VnVInputFile.DEFAULT_SPEC.copy()

        return r

    def setConnection(self, hostname, username, password, port):
        self.connection = VnVConnection(hostname, username, password, port)

    def setConnection(self):
        self.connection = VnVLocalConnection()

    def setFilename(self, fname):
        self.filename = fname
        self.updateSpec()

    def getFileStatus(self):
        if self.connection.connected():
            if self.connection.exists(self.filename):
                return ["green", "Valid"]
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

        self.value = newValue
        # if the plugins are different, try to reload the specification.
        if self.plugs is None or self.plugs != self.getPlugins(newValue):
            self.updateSpec()

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

    def saveSpec(self, auto, dump, value):
        self.specAuto = auto
        if auto:
            self.specDump = dump
            self.updateSpec()
        else:
            self.spec = value
            try:
                self.specLoad = json.loads(self.spec)
            except:
                self.specLoad = VnVInputFile.DEFAULT_SPEC.copy()

        return self.spec

    def updateSpec(self):

        def getSpecDumpCommand(inputfilename):
            main = self.specDump.replace("${application}", self.filename)
            main = main.replace("${inputfile}", inputfilename)
            return main

        if self.specAuto:
            try:
                aa = json.loads(self.value)
                s = {"additionalPlugins": aa.get("additionalPlugins", {})}
            except:
                s = {"additionalPlugins": {}}

            s["schema"] = {"dump": True, "quit": True}
            path = self.connection.write(json.dumps(s), None)

            a = getSpecDumpCommand(path)
            try:
                res = self.connection.execute(a)
                a = res.find("===START SCHEMA DUMP===") + len("===START SCHEMA DUMP===")
                b = res.find("===END SCHEMA_DUMP===")
                if a > 0 and b > 0 and b > a:
                    self.spec = res[a:b]
                    try:
                        self.specLoad = json.loads(self.spec)
                    except:
                        self.specLoad = VnVInputFile.DEFAULT_SPEC.copy()
            except Exception as e:
                # Cant update spec unless the input file is at least valid json.
                # It also won't update if the input file is invalid.
                print(e)

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
            "name" : {"type" : "string"},
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
    @staticmethod
    def getExecutionSchema():
        if VnVInputFile.EXECUTION_SCHEMA is None:
            VnVInputFile.EXECUTION_SCHEMA = json.loads(json.dumps(VnVInputFile.CONFIG_SCHEMA))
            VnVInputFile.EXECUTION_SCHEMA["properties"]["active_overrides"] = {"type": "array", "items": {"type" : "string"} }
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
                "name" : "Hello"
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

    def autocomplete_input(self, row, col, pre, val):
        return autocomplete(val, self.specLoad, int(row), int(col))

    def autocomplete_exec(self, row, col, pre, val):
        return autocomplete(val, VnVInputFile.getExecutionSchema(), int(row), int(col))

    def autocomplete_spec(self, row, col, pre, val):
        return []

    def get_jobs(self):
        return [ a for a in self.connection.get_jobs()]

    def execute(self, val, ):
        script, name = self.script(val)
        return self.connection.execute_script(script, name=name, vnv_input=self.value)

    def script(self, val):
        data = json.loads(val)
        for i in data.get("active_overrides",[]):
            if i in data.get("overrides",{}):
                over = data["overrides"][i]
                for k,v in over.items():
                    data[k] = v

        return bash_script(self.filename,self.value, data), data.get("name")

    @staticmethod
    def get_id():
        VnVInputFile.COUNTER += 1
        return VnVInputFile.COUNTER

    @staticmethod
    def add(name, path=None):

        a = mongo.loadInputFile(name)
        if a is not None:
            raise Exception("Name is taken")
            f = VnVInputFile.fromJson(a)
        else:
            f = VnVInputFile(name,path=path)

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

def bash_script(application_path, inputfile, data):
    return textwrap.dedent(f"""
{data.get("shebang", "#!/usr/bin/bash")}

export application={application_path}
export application_dir=$(dirname {application_path})
export inputfile={data.get("input-file-name",".vv-input.json")}

cd {data.get("working-directory", "${application_dir}")}

cat << EOF > ${{inputfile}}
    {inputfile}
EOF

{njoin(["export " + k + "=" + v for k, v in data.get("environment", {})])}
{njoin([a["action"] + " " + a["source"] + " " + a["dest"]] for a in data.get("input-staging", {}))}
{data.get("command-line", "")}
{njoin([a["action"] + " " + a["source"] + " " + a["dest"]] for a in data.get("output-staging", {}))} 
""")