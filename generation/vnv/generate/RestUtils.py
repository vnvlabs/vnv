﻿#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import shutil
from io import StringIO
import hashlib


def writeStream(stream, text, newLine=True):
    stream.write(text)
    if newLine:
        stream.write("\n")


headerMap = {
    "title": ("*", True),
    "sub": ("#", False),
    "subsub": ("*", False),
    "subsubsub": ("\"", False),
    "para": (",", False)
}


def createOutputDirectory(outputDirectory):
    if not os.path.exists(outputDirectory):
        os.makedirs(outputDirectory)
    return os.path.abspath(outputDirectory)


def writeFile(rootdir, filename, text, ext=".rst", textName=None):
    absD = createOutputDirectory(rootdir)
    fullname = os.path.join(absD, filename + ext)

    # TODO should do this with the app.env storage, but this will do for now
    # Not even sure if this speeds it up or not.
    if os.path.exists(fullname):
        oldt = hashlib.md5(open(fullname, "rb").read()).hexdigest()
        newt = hashlib.md5(bytes(text, "ascii")).hexdigest()
        if newt == oldt:
            return [textName, filename] if textName else filename

    with open(fullname, "w") as f:
        f.write(text)
    if textName is None:
        return filename
    else:
        return [textName, filename]


def readFile(filename):
    with open(filename, 'r') as f:
        return f.read()


def indent(count):
    return " " * count


def getHeader(title, type="title", ref=None):
    if type not in headerMap:
        raise RuntimeError("Type Does not exist")
    stream = StringIO()

    if ref is not None:
        writeStream(stream, ".. _{}:".format(ref))

    sign, over = headerMap[type]
    if over:
        writeStream(stream, sign * len(title))
    writeStream(stream, title)
    writeStream(stream, sign * len(title))
    return stream.getvalue()


def it(word):
    return "*" + word.lstrip() + "*"


def bf(word):
    return it(it(word))


def link(link, linkText):
    return "`{linkText} <{link}>`_".format(linkText=linkText, link=link)


def implicitTitleLink(title):
    return "`{title}`_".format(title)


def explicitTitleLink(refName, text=False):
    if text:
        return "{}_".format(refName)
    return ":ref:`{}`".format(refName)


def verbatim(text):
    return "``{text}``".format(text=text)


def tocTree(files, **kwargs):
    stream = StringIO()
    # kwargs["maxdepth"] = 2
    # kwargs["hidden"] = ""
    # kwargs["includehidden"] = ""

    writeStream(stream, ".. toctree::")
    for i in kwargs:
        if i in [
            "maxdepth",
            "numbered",
            "titlesonly",
            "glob",
            "hidden",
                "includehidden"]:
            writeStream(stream, "  :{}: {}".format(i, kwargs[i]))
        else:
            raise RuntimeError("Invalid Toc Tree Parameter {}".format(i))
    count = 0
    writeStream(stream, "\n")
    for file in files:
        if isinstance(file, list):
            writeStream(stream, "  {} <{}>".format(file[0], file[1]))
            count += 1
        elif file is not None:
            writeStream(stream, "  {}".format(file))
            count += 1
    return stream.getvalue() if count > 0 else ""


def htmllist(values, numbered=False):
    sym = "#" if numbered else "*"
    stream = StringIO()
    for item in values:
        for n, v in enumerate(item.splitlines()):
            ssym = sym if n == 0 else " "
            writeStream(stream, "{sym} {val}".format(sym=ssym, val=v))
    return stream.getvalue()


def codeBlock(text):
    stream = StringIO()
    writeStream(stream, "::")
    for i in text.splitlines():
        writeStream(stream, "  {}".format(i))
    return stream.getvalue()


def startCollapse():
    stream = StringIO()
    stream.write("\n\n.. raw: html \n\n ")
    stream.write("    <details>\n\n")
    return stream.getvalue()


def stopCollapse():
    stream = StringIO()
    stream.write("\n\n.. raw: html \n\n ")
    stream.write("    </details>\n\n")
    return stream.getvalue()


def image(filename, **kwargs):
    stream = StringIO()

    type = "image" if kwargs.get('figclass', None) is None else "figure"
    writeStream(stream, ".. {type}:: {file}".format(file=filename, type=type))

    for i in kwargs:
        if i in ["align", "height", "width", "alt", "figclass"]:
            writeStream(stream, "  :{}: {}".format(i, kwargs[i]))
        else:
            raise RuntimeError("Invalid Image/Figure property {}".format(i))
    return stream.getvalue()


def box(text, type="seealso"):
    if type not in ["seealso", "warning", "note"]:
        raise RuntimeError("Invalid Box Type {}".format(type))
    return ".. {type}:: {text}".format(type=type, text=text)


if __name__ == "__main__":
    try:
        print("Fail", getHeader("falillsldf", "fail"))
    except BaseException:
        print("Fail Header Test Passed")
    print("Box:\n", box("sdfsdfsdfSDf"))
    print("Box Note: \n", box("sdfsdfsdfSDf", type="note"))
    print("Box Warning : \n", box("sdfsdfsdfSDf", type="warning"))
    try:
        print("Fail Box: \n", box("sdfsdf", type="sdfsf"))
    except BaseException:
        print("Faid Box type Text Passed")

    tocTreeD = [["sdfs", "ssss"], "sdfsdf", ["sfsdfsdf", "sss"]]
    print(tocTree(tocTreeD))

    print("Image: \n", image("sdfsdf.jpg", align="center", width="100px"))

    print(htmllist(["sdfsdf", "sfgsgdfdhdh", "sssssssss"]))

    print(codeBlock("sdfsdfsdfsdfsdf\nsfsdfsdfsdf\nsfsdsssssssssss"))
    print(r"SDFSDF\n\n\n\SDFSDFF")
