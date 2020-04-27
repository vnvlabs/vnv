#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import json
import io

from . import configs
from . import utils
## Take a list of json files, or dictionaries and load them into
## a single dictionary. In case of collisions, we will take the last
## entry.

def generateAllTreeFiles():
    for f in configs.getOutputFiles().values:
          f.generateTree()

def explode():
    configs.generateAllTreeFiles()
    configs._the_app.vnvtree_root = configs.getOutputFiles()

