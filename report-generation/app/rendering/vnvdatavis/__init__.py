import os
from sphinx.errors import ConfigError
from . import directives

def list_assets():
    return [
        os.path.join(
            os.path.dirname(__file__),
            a) for a in (
            directives.vnv_css_assets +
            directives.vnv_js_assets)]


def setup(app):
    from . import directives
    app.add_config_value("vnv_file", -1, "env")
    directives.setup(app)  # Setup the generator vnvdatavis
    return {'version': 1.0}
