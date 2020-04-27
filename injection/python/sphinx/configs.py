#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import os
import io
import json
from vnv import VnVReader as OR
import importlib
from basic_tree.VnVTreeGenerator import VnVTreeGenerator

from sphinx.errors import ConfigError, ExtensionError
from sphinx.util import logging
from types import FunctionType, ModuleType

try:
    # Python 2 StringIO
    from cStringIO import StringIO
except ImportError:
    # Python 3 StringIO
    from io import StringIO

containmentFolder = "./vnvtree" # The folder to write the restructured text to.
rootFileName = "index.rst" # The filename for the root file.
rootFileTitle = "VnV Output File" # The title of the root file.

_the_app = None
_app_src_dir = None

_search_string_json = None
outputFiles = {} # map of OutputFileConfig objects for us to parse
_outputFiles = {} # map of output file configs that can be used, indexed by name.


class OutputFileConfig:

     # Class for specifying the Output file config.
     def __init__(self, name, filename, reader, outputDir, **kwargs)
        self.name = name
        self.filename = filename
        self.reader = reader
        self.outputDir = outputDir
        self.readerConfig = kwargs.get("readerConfig",{})
        self.generator = kwargs.get("generator",None)
        self.generatorConfig = kwargs.get("generatorConfig",{})
        self.generatorImpl = None
        self.readerImpl = None

     def getReader(self):
          if self.readerImpl is None:
               self.readerImpl = OR.ReaderWrapper(self.reader,self.filename, json.dumps(self.readerConfig))
          return self.readerImpl

     def getGenerator(self):
          if self.generatorImpl is None:
               if self.generator is None:
                    self.generatorImpl = BasicTreeGenerator()
                    self.generatorImpl.config(**self.generatorConfig)
               elif isinstance(self.generator,str):
                    self.generatorImpl =  OR.TreeWrapper(self.generator, json.dumps(self.generatorConfig))
               elif isinstance(self.generator,VnVTreeGenerator):
                    self.generatorImpl = self.generator
                    self.generatorImpl.config(**self.generatorConfig)
               else:
                    raise RuntimeError("Invalid Generator Specification")
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
          return os.path.join(containmentFolder,self.config.name + "-tree")

     def generateTree(self):
          gen = self.getGenerator()
          if gen is not None:
               root = self.getRootNode()
               if root is not None:
                    gen.generateTree(self.getOutputDirectory(), self.getRootNode())






def getOutputFiles():
     return _outputFiles

def apply_sphix_configuration(app):
     from . import deploy
     from . import utils

     args = app.config.vnvtree_args

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

     
     #Initialize The VnV Library
     OR.Initialize(vnvargs, args.get("vnvConfigurationFile"))


     for file in outputFiles:
          f = outputFiles[file]
          if isinstance(f,OutputFileConfig):
               _outputFiles[file] = OutputFileImpl(f)
          else:
               raise ConfigError("Only instance of OutputFileConfig can be specified")
