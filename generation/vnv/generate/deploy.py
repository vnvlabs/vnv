#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from . import configs
from .basic_tree import VnVTreeGenerator

## Take a list of json files, or dictionaries and load them into
## a single dictionary. In case of collisions, we will take the last
## entry.

def explode():
    if configs.generateFiles:
       print("Re generating All files")
       VnVTreeGenerator.generateAllTrees(configs.getOutputFiles(), configs.containmentFolder, configs.rootFileName)
    else:
       print("skipping document generation")
    configs._the_app.vnvtree_root = configs.getOutputFiles()

