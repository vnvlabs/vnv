

def setup(sapp):
    from . import general, apexcharts, dashboard
    general.setup(sapp)
    apexcharts.setup(sapp)
    dashboard.setup(sapp)
