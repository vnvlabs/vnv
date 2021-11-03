from app.rendering.vnvdatavis.directives import basic, line,charts,jmes,dashboard,gauge,table, psip, issues


def setup(sapp):
    jmes.setup(sapp)
    charts.setup(sapp)
    basic.setup(sapp)
    line.setup(sapp)
    dashboard.setup(sapp)
    gauge.setup(sapp)
    table.setup(sapp)
    psip.setup(sapp)
    issues.setup(sapp)