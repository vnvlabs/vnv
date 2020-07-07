#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals
__version__ = "0.0.1"

# Functions to hook up as a sphinx extension.
def environment_ready(app):
    # Defer importing configs until sphinx is running.
    from . import configs
    configs.apply_sphinx_configurations(app)

def setup(app):
    app.add_config_value("vnv_args", {}, "env")
    app.connect("builder-inited", environment_ready)
    return {"version": __version__}
