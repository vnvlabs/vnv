from . import FunctionDirective
from . import ParaviewDirective


def setup(app):
    FunctionDirective.setup(app)
    ParaviewDirective.setup(app)
