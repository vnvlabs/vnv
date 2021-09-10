import json
import os
import subprocess
import uuid

import python_api.VnVReader as VnVReader

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
            "type": "json_stdout",
            "config": {}
        },
        "injectionPoints": [
        ]
    }


initialized = False


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
    return json.loads(VnVReader.VnVDumpReaders())


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
    print(config)
    return VnVReader.Read(filename, reader, json.dumps(config))
