from .charts import ChartJsDirective
from .paraview import ParaviewDirective
from .table import TableDirective
from .jmes import JmesDirective

# Global setup function for loading all the directives. 
def setup(app):
   ChartJsDirective.setup(app)
   ParaviewDirective.setup(app)
   TableDirective.setup(app)
   JmesDirective.setup(app)


