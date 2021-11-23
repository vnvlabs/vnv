import json
import os
import sys
from datetime import datetime
from pathlib import Path

def crumb(dir):
    c = os.path.normpath(dir).split(os.path.sep)
    cc = os.path.abspath(os.sep)
    loc = []
    for i in c:
        if len(i):
            cc = os.path.join(cc, i)
            loc.append(cc)
    return loc


def info(path):

    inf = {}
    abspath = os.path.abspath(path)

    inf["abspath"] = abspath

    inf["dir"] = os.path.dirname(abspath)
    inf["isdir"] = os.path.isdir(abspath)
    inf["name"] = os.path.basename(abspath)
    inf["home"] = os.path.expanduser("~")
    inf["root"] = os.path.abspath(os.sep)
    inf["exists"] = os.path.exists(abspath)
    inf["children"] = [os.path.join(abspath, i) for i in os.listdir(abspath)] if inf["exists"] and inf["isdir"] else []
    inf["size"] = os.lstat(abspath).st_size if len(abspath) and inf["exists"] else 0
    inf["lastMod"] = os.lstat(abspath).st_mtime if len(abspath) and inf["exists"] else 0
    inf["lastModStr"] = (datetime.fromtimestamp(inf["lastMod"]).strftime('%Y-%m-%d %H:%M:%S')) if len(abspath) and inf["exists"] else ""
    inf["ext"] = "directory" if inf["exists"] and Path(abspath).is_dir() else os.path.splitext(abspath)[1]
    inf["crumb"] = crumb(abspath)
    return inf

def get_file_name():
    return os.path.relpath(__file__)

if __name__ == "__main__":
    print(json.dumps(info(sys.argv[1])))
    exit(0)