import json

from sphinx.errors import ExtensionError

from app.rendering.vnvdatavis.directives.charts import PlotlyChartDirective

class PlotlyChart(PlotlyChartDirective):

    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = FakeDict()

    def getData(self):


    def getContent(self):
        return json.dumps(self.getData())


try:
    the_app
except NameError:
    the_app = None


def setup(sapp):
    global the_app
    the_app = sapp

    for key, value in directives.items():
        sapp.add_directive(key, value)
