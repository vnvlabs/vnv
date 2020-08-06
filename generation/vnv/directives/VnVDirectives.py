from .charts import ChartJsDirective
from . import paraview
from .jmes import JmesDirective
from .nodes import VnVNodes


# Global setup function for loading all the directives.
def setup(app):
    ChartJsDirective.setup(app)
    paraview.setup(app)
    JmesDirective.setup(app)
    VnVNodes.setup(app)
