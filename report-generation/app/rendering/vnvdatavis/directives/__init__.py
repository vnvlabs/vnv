

def setup(sapp):

    from . import jmes, charts, basic, apexcharts, dashboard, gauge

    jmes.setup(sapp)
    charts.setup(sapp)
    basic.setup(sapp)
    apexcharts.setup(sapp)
    dashboard.setup(sapp)
    gauge.setup(sapp)

