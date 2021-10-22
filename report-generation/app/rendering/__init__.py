import os
import shutil
import textwrap

import sphinx.cmd.build


conf_template = '''
import sys
sys.path.append("{path}")

project = "VNV"
copyright = f"2021, VNV"
author = "VNV"
release = "VNV"
extensions = [
    "sphinx.ext.mathjax",
    "vnvdatavis"
]
vnv_file={fileId}
html_theme = "faketheme"
html_theme_path = ["{path}"]
exclude_patterns = ['_build']
html_static_path = ['_static']

'''
def get_conf_template(fileId):
    return conf_template.format(fileId=fileId, path=os.path.abspath(os.path.join('app', 'rendering')))


def setup_build_directory(src_dir, fileId):
    os.makedirs(os.path.join(src_dir, "_build"))
    os.makedirs(os.path.join(src_dir, "_static"))
    os.makedirs(os.path.join(src_dir, "_templates"))

    # Write a configuration file in the source directory.
    with open(os.path.join(src_dir, "conf.py"), 'w') as w:
        w.write(get_conf_template(fileId))

    # Override the template with a simple template that just shows the body.

# files dict ( type -> package -> name -> template


class TemplateBuild:
    def __init__(self, src_dir, id_):
        self.src_dir = src_dir
        self.file = id_

    def get_html_file_name(self, type, package, name):
        return os.path.join(
            "renders",
            self.src_dir,
            "_build",
            "html",
            f"{type}_{package}_{name}.html")

    def get_action(self, package, name):
        return self.get_html_file_name("Actions", package, name)

    def get_unit_test_test_content(self, package, name, test):
        return os.path.join("renders",self.src_dir, "_build", "html", f"UnitTest_Test_{package}_{name}_{test}.html")

    def get_package(self, package):
        return os.path.join(
            "renders",
            self.src_dir,
            "_build",
            "html",
            f"Options_{package}.html")

    def get_assets_directory(self):
        return os.path.join(
            "renders",
            self.src_dir,
            "_build",
            "html",
            "static")

    def get_introduction(self):
        return os.path.join(
            "renders",
            self.src_dir,
            "_build",
            "html",
            "Introduction.html")

    def get_conclusion(self):
        return os.path.join(
            "renders",
            self.src_dir,
            "_build",
            "html",
            "Conclusion.html")

    def get_file_description(self, package, name):
        return self.get_html_file_name("Files", package, name)


def build(src_dir, templates, id_):
    setup_build_directory(src_dir, id_)

    fnames = []
    for type_, packages in templates.items():
        if type_ in ["Introduction"]:
            fnames.append(f"{type_}.rst")
            with open(os.path.join(src_dir, fnames[-1]), 'w') as w:
                w.write(f'{textwrap.dedent(packages["docs"])}\n\n')
        elif type_ in ["Options"]:
            for package, docinfo in packages.items():
                fnames.append("Options_" + package + ".rst")
                with open(os.path.join(src_dir, fnames[-1]), 'w') as w:
                    w.write(f'\n\n{textwrap.dedent(docinfo["docs"])}\n\n')

        elif type_ not in ["Conclusion"]:
            for package, point in packages.items():
                name_package = package.split(":")
                fnames.append(
                    f"{type_}_{name_package[0]}_{name_package[1]}.rst")

                with open(os.path.join(src_dir, fnames[-1]), 'w') as w:
                    if len(point["docs"]) == 0:
                        w.write(f"\n\n<No information available>\n\n")
                    else:
                        w.write(f"\n{textwrap.dedent(point['docs'])}\n\n")

                if type_ == "UnitTests":
                    tests = point["tests"]
                    for test in tests.keys() :
                        fnames.append(f"UnitTest_Test_{name_package[0]}_{name_package[1]}_{test}.rst")
                        with open(os.path.join(src_dir, fnames[-1]), 'w') as w:
                            if len(point["docs"]) == 0:
                                w.write(f"\n\n<No information available>\n\n")
                            else:
                                w.write(f"\n{textwrap.dedent(tests[test])}\n\n")

        else:
            fnames.append(f"{type_}.rst")
            with open(os.path.join(src_dir, fnames[-1]), 'w') as w:
                w.write(f'\n\n{textwrap.dedent(packages["docs"])}\n\n')

    index = '''.. toctree::\n\t:maxdepth: 22\n\t:caption: Contents:\n\n\t{files}\n\n'''.format(
        files="\n\t".join(fnames))
    with open(os.path.join(src_dir, "index.rst"), 'w') as w:
        w.write(index)

    params = ["-M", "html", src_dir, os.path.join(src_dir, "_build")]
    sphinx.cmd.build.make_main(params)

    return TemplateBuild(os.path.basename(src_dir), id_)
