import os
from sphinx.errors import ConfigError
from . import directives

_the_app = None
_app_src_dir = None


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


def environment_ready(app):
    apply_sphinx_configurations(app)


def list_assets():
    return [
        os.path.join(
            os.path.dirname(__file__),
            a) for a in (
            directives.vnv_css_assets +
            directives.vnv_js_assets)]


def setup(app):
    from . import directives
    app.add_config_value("vnv_args", {}, "env")
    app.connect("builder-inited", environment_ready)
    directives.setup(app)  # Setup the generator vnvdatavis
    return {'version': 1.0}
