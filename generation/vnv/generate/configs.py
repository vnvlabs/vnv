#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import os
import io
import json
import importlib
import shutil
import tempfile

import jmespath

from ..vnv import VnVReader as OR
from .basic_tree.VnVTreeGenerator import VnVTreeGenerator

from sphinx.errors import ConfigError, ExtensionError
from sphinx.util import logging
from types import FunctionType, ModuleType
from io import StringIO

generateFiles = False
# The folder to write the restructured text to.
containmentFolder = "./vnvtree"
rootFileName = "index"  # The filename for the root file.
rootFileTitle = "VnV Output File"  # The title of the root file.
generator = VnVTreeGenerator()

_the_app = None
_app_src_dir = None

vnv_plugins = {}

js_dir = "_static/js/"


def add_js_files(outdir, files, srcdir):
    # Make the output directory
    print(outdir, files, srcdir)
    jsd = os.path.join(outdir, js_dir)

    print(outdir, js_dir, os.path.join(outdir, js_dir))
    if not os.path.exists(jsd):
        os.makedirs(jsd)

    # Check the source directory exists.
    if not os.path.isdir(srcdir):
        raise ExtensionError("Cannot file data directory")
    for f in files:
        absPath = os.path.join(srcdir, f)
        if not os.path.exists(absPath):
            raise ExtensionError("Cannot find file: " + absPath)
        shutil.copy(absPath, jsd)


def getVnVConfigFile():
    return {
        "runTests": True,
        "runScopes": [
            {
                "name": "all",
                "run": True
            }
        ],
        "logging": {
            "on": True,
            "filename": "stdout",
            "logs": {}
        },
        "additionalPlugins": vnv_plugins,
        "outputEngine": {
            "type": "json",
            "config": {}
        },
        "injectionPoints": [
        ]
    }


def getApp():
    return _the_app


def getGenerator():
    return generator


def apply_sphinx_configurations(app):
    args = app.config.vnv_args

    global _the_app
    _the_app = app

    global _app_src_dir
    _app_src_dir = os.path.abspath(app.srcdir)

    configs_globals = globals()
    for key in args:
        if key in configs_globals:
            configs_globals[key] = args[key]
        else:
            raise ConfigError("Unrecognized Key: " + key)

    OR.Initialize([], getVnVConfigFile())
