
def setup(sapp):

    from . import basic, line, charts, jmes, dashboard, gauge, table, psip, issues, \
        plotly, iff, slider, forr, plotly_animation, apex, chartsjs, include, math

    jmes.setup(sapp)
    charts.setup(sapp)
    basic.setup(sapp)
    line.setup(sapp)
    dashboard.setup(sapp)
    gauge.setup(sapp)
    table.setup(sapp)
    psip.setup(sapp)
    issues.setup(sapp)
    plotly.setup(sapp)
    iff.setup(sapp)
    slider.setup(sapp)
    forr.setup(sapp)
    plotly_animation.setup(sapp)
    apex.setup(sapp)
    chartsjs.setup(sapp)
    include.setup(sapp)
    math.setup(sapp)