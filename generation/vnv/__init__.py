#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals
from .directives import VnVDirectives
from .generate import Generate
__version__ = "0.0.1"



def setup(app):
    Generate.setup(app) # Setup the generator extensions
    VnVDirectives.setup(app) # Setup the ResT extensions 
    return {"version": __version__}
