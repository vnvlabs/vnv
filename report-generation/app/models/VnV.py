import json
import os
import subprocess
import uuid

import python_api.VnVReader as VnVReader
from app.base.utils import mongo

THISDIR = os.path.dirname(os.path.abspath(__file__))

def getVnVConfigFile():
    return {
        "runTests": True,
        "outputEngine": {
            "null" : {}
        }
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

    #### TODO get this info from the a above.
    a = [
        ["json_file", "Enter the directory used to initialize the json file engine.", "file"],
        ["adios_file", "Enter the directory name used when initializing the adios file engine.", "file"],
        ["json_socket", "Enter the port to launch the socket server on on.", "integer"],
        ["json_http", "Enter the port to launch the http server on", "integer"]
    ]
    if mongo.Configured():
        a.append(["saved", "Enter the name of the collection to load", "collection"])

    return a


def Read(filename, reader, config={}):
    Intialize()
    return VnVReader.Read(filename, reader, json.dumps(config))
