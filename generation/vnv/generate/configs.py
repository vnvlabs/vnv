﻿#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import os
import io
import json
import importlib
import jmespath

from ..vnv import VnVReader as OR
from .basic_tree.VnVTreeGenerator import VnVTreeGenerator

from sphinx.errors import ConfigError, ExtensionError
from sphinx.util import logging
from types import FunctionType, ModuleType
from io import StringIO

generateFiles = False
containmentFolder = "./vnvtree"  # The folder to write the restructured text to.
rootFileName = "index"  # The filename for the root file.
rootFileTitle = "VnV Output File"  # The title of the root file.
generator = VnVTreeGenerator()

_the_app = None
_app_src_dir = None

vnv_plugins = {}


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
    "on" : True,
    "filename" : "stdout",
    "logs" : {}
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
