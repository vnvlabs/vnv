from flask import render_template


def render_error(code, message):
    return render_template(
        'page-custom.html', message=message, code=code), code
