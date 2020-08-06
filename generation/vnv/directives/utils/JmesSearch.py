import jmespath
import os
import uuid
import shutil

from ..utils import RootNodeVisitor
from ...vnv import VnVReader


def getUrl(filepath, outDir, env, writer=None):
    if not hasattr(env, "vnv_copied_files"):
        env.vnv_copied_files = {}

    if filepath.startswith('http') or filepath.startswith('ftp'):
        return filepath
    else:

        if not os.path.exists(filepath):
            raise RuntimeError(
                "File {} needed does not exist".format(filepath))

        if filepath in env.vnv_copied_files:
            url = env.vnv_copied_files[filepath]
        else:

            ext = os.path.splitext(filepath)[1]
            url = os.path.join('_static/files', str(uuid.uuid4()) + ext)
            while os.path.exists(os.path.join(outDir, url)):
                url = os.path.join('_static/files', str(uuid.uuid4()) + ext)
            env.vnv_copied_files[filepath] = url

        fname = os.path.join(outDir, url)
        if not os.path.exists(fname):
            if not os.path.exists(os.path.dirname(fname)):
                os.makedirs(os.path.dirname(fname))

            if writer is None:
                shutil.copy(filepath, fname)
            else:
                writer(filepath, fname)
        return "/" + url  # Make it relative to the static dir.


# Get a raw file path
def getFilePath(filename, node=None, srcdir=None):
    if filename.startswith(
            "http://") or filename.startswith("https://") or filename.startswith("ftp://"):
        return filename

    if filename.startswith("vnv:"):
        jmes = filename[4:]
        return str(getJMESNode(node, jmes))

    elif filename.startswith("." + os.path.sep) or filename.startswith(".." + os.path.sep):
        if srcdir is None:
            raise RuntimeError(
                "No source dir available for relative file reference.")
        return os.path.join(srcdir, filename)
    else:
        return filename


def getJMESNode(inode, jmesString):
    node = VnVReader.castDataBase(inode)
    expression = jmespath.compile(jmesString)
    print("STRING ", jmesString)
    print("NODE", node)
    result = RootNodeVisitor.search(expression, node)
    print("RESULT", result)
    return result
