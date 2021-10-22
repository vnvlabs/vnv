import os
from sphinx.errors import ConfigError
from . import directives


def setup(app):
    from . import directives
    app.add_config_value("vnv_file", -1, "env")
    directives.setup(app)  # Setup the generator vnvdatavis
    return {'version': 1.0}
