
vnv_roles = {}
vnv_directives = {}
vnv_nodes = []
vnv_js_assets = []
vnv_css_assets = []


def setup(app):
    import rendering.vnvdatavis.directives.apexcharts
    import rendering.vnvdatavis.directives.general

    for node in vnv_nodes:
        app.add_node(node, **node.NODE_VISITORS)
    for key, value in vnv_roles.items():
        app.add_role(key, value)
    for key, value in vnv_directives.items():
        app.add_directive(key, value)
    for file in vnv_js_assets:
        app.add_js_file(file)
    for file in vnv_css_assets:
        app.add_css_file(file)
