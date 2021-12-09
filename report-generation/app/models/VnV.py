import json
import os
import subprocess
import uuid

import python_api.VnVReader as VnVReader
from app.base.utils import mongo

THISDIR = os.path.dirname(os.path.abspath(__file__))
SPECDIR = os.path.join(THISDIR, "specs")
if not os.path.exists(SPECDIR):
    os.mkdir(SPECDIR)


def getVnVConfigFile():
    return {
        "runTests": True,
        "logging": {
            "on": True,
            "filename": "stdout",
            "logs": {}
        },
        "additionalPlugins": {},
        "outputEngine": {
            "type": "null",
            "config": {}
        },
        "injectionPoints": [
        ]
    }


initialized = False

FILES = {}


def Intialize():
    global initialized
    if not initialized:
        print(getVnVConfigFile())
        VnVReader.Initialize([], getVnVConfigFile())
        initialized = True


def Finalize():
    global initialized
    if initialized:
        VnVReader.Finalize()
        initialized = False


def LoadPlugin(name, filename):
    Intialize()
    VnVReader.LoadPlugin(name, filename)


def DumpReaders():
    Intialize()
    a = json.loads(VnVReader.VnVDumpReaders())
    a.append("Pipeline")

    #### TODO get this info from the a above.
    a = [
        ["json_file", "Enter the directory used to initialize the json file engine.", "file"],
        ["adios_file", "Enter the directory name used when initializing the adios file engine.", "file"],
        ["pipeline", "Enter the name of the pipeline file.", "file"],
        ["json_socket", "Enter the port to launch the socket server on on.", "integer"],
        ["json_http", "Enter the port to launch the http server on", "integer"]
    ]
    if mongo.Configured():
        a.append(["saved", "Enter the name of the collection to load", "collection"])

    return a

def LoadSpec(application):
    t = None
    while t is None or os.path.exists(t):
        t = os.path.join(SPECDIR, uuid.uuid4().hex)
    with open(t, 'w') as f:
        f.write(json.dumps(getVnVConfigFile()))

    u = None
    while u is None or os.path.exists(u):
        u = os.path.join(SPECDIR, uuid.uuid4().hex)

    subprocess.run([application, "--vnv-qdump", u, "--vnv-input-file", t])

    with open(u, 'r') as ff:
        return json.load(ff)


def Read(filename, reader, config={}):
    Intialize()
    return VnVReader.Read(filename, reader, json.dumps(config))
