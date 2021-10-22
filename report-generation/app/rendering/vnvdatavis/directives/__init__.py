from . import jmes, charts, basic, apexcharts, dashboard, gauge


def setup(sapp):
    jmes.setup(sapp)
    charts.setup(sapp)
    basic.setup(sapp)
    apexcharts.setup(sapp)
    dashboard.setup(sapp)
    gauge.setup(sapp)

