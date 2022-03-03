
def setup(app):
    from . import directives
    app.add_config_value("vnv_file", -1, "env")
    app.add_config_value("update_dir","","html")
    directives.setup(app)  # Setup the generator vnvdatavis
    return {'version': 1.0}
