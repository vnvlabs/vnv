#!/usr/bin/env python
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

def getVnVConfigFile():
  {
  "runTests": True,
  "runScopes": [
    {
      "name": "all",
      "run": True
    }
  ],
  "logging": {
        "on" : False,
    "filename" : "stdout",
    "logs" : {}
  },
  "testLibraries": vnv_plugins,
  "outputEngine": {
    "type": "json",
    "config": {}
  },
  "injectionPoints": []
  }

generateFiles = False
containmentFolder = "./vnvtree" # The folder to write the restructured text to.
rootFileName = "index" # The filename for the root file.
rootFileTitle = "VnV Output File" # The title of the root file.

#Path required if the file was written using a plugin reader.
vnv_plugins = {}

_the_app = None
_app_src_dir = None

_search_string_json = None

files = {}
_outputFiles = {} # map of output file configs that can be used, indexed by name.
_outputFiles_ = {}

class OutputFileConfig:
     # Class for specifying the Output file config.
     def __init__(self, name, filename, reader, outputDir, **kwargs):
        self.name = name
        self.filename = filename
        self.outputDir = outputDir
        self.reader = reader
        self.readerConfig = kwargs.get("readerConfig",{})
        self.generatorConfig = kwargs.get("generatorConfig",{})
        self.generatorImpl = None
        self.readerImpl = None
        self.generatorConfig["__jmes_root__"] = self.name 


     def getReader(self):
          if self.readerImpl is None:
               self.readerImpl = OR.Read(self.filename,self.reader,json.dumps(self.readerConfig));
          return self.readerImpl

     def getGenerator(self):
          if self.generatorImpl is None:
               self.generatorImpl = VnVTreeGenerator()
               self.generatorImpl.config(**self.generatorConfig)
          return self.generatorImpl


class OutputFileImpl:

     def __init__(self, config):
          self.config = config
          self.reader = self.config.getReader()
          self.rootNode = None
          self.generator = None

     def getRootNode(self):
          if self.rootNode is None:
               self.rootNode = self.reader.get()
          return self.rootNode

     def getGenerator(self):
          if self.generator is None:
               self.generator = self.config.getGenerator()
          return self.generator

     def getOutputDirectory(self):
          return os.path.join(containmentFolder,self.config.name)

     def generateTree(self):
          gen = self.getGenerator()
          if gen is not None:
               root = self.getRootNode()
               if root is not None:
                    toc = gen.generateTree(self.getOutputDirectory(), self.getRootNode())
                    if isinstance(toc,str):
                         return os.path.join(self.config.name,toc)
                    else:
                         return [toc[0], os.path.join(self.config.name,toc)]


def getTextForNode(node):
     return node.__str__()



def getJmesSearchNode(jmesString):
     jmes = jmesString.split(".")
     if len(jmes) == 0:
          return { (i,j.getRootNode()) for i,j in _outputFiles.iteritems() }
     if jmes[0] not in _outputFiles:
          raise RuntimeError("No File Loaded with rootName {}".format(jmes[0]))
     return {jmes[0] : _outputFiles.get(jmes[0]).getRootNode()}


def getOutputFiles():
     return _outputFiles


def apply_sphinx_configurations(app):
     from . import deploy

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
               raise ConfigError("Unreconized Key: " + key)

     OR.Initialize([], getVnVConfigFile())

     # TODO Generation only really needs to happen for a few
     # reasons (1) the file changes. (2) the templates change
     # (the options change).
     for f in files:
         d = files[f]
         if "filename" not in d:
             raise ConfigError("No filename given for {}".format(f))
         name = f
         filename = d["filename"]
         reader = d.get("reader",None)
         outDir = d.get("outDir",name + "-output")
         args = d.get("args",{})
         conf = OutputFileConfig(name,filename, reader, outDir, **args)
         _outputFiles[name] = OutputFileImpl(conf) 


