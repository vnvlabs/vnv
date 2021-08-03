"""The VnV setup module.
"""

# Always prefer setuptools over distutils
from setuptools import setup, find_packages
import pathlib

here = pathlib.Path(__file__).parent.resolve()

# Get the long description from the README file
long_description = (here / 'README.md').read_text(encoding='utf-8')


setup(
    name='vnv-python-binding',  # Required

    version='0.0.1',  # Required

    description='VnV python API for reading VnV files',  # Optional

    long_description=long_description,  # Optional

    long_description_content_type='text/markdown',  # Optional (see note above)

    url='https://gitlab.rnet-tech.net/boneill/vv-neams.git',  # Optional

    author='RNET Technologies',  # Optional

    author_email='boneill@rnet-tech.com',  # Optional

    classifiers=[  # Optional
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Build Tools',


        # Specify the Python versions you support here. In particular, ensure
        # that you indicate you support Python 3. These classifiers are *not*
        # checked by 'pip install'. See instead 'python_requires' below.
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3 :: Only',
    ],

    keywords='RNET, VnV, Verification and Validation, Sphinx, Report Generation',  # Optional


    packages=find_packages(),  # Required

    python_requires='>=3.5, <4',

    install_requires=[
        'sphinx',
        'sphinx_rtd_theme',
        'selenium',
        'scikit-image',
        'jmespath'],
    # Optional

    extras_require={  # Optional
    },

    package_data={  # Optional
        "vnv.vnv": ["*.so"]
    },


    # To provide executable scripts, use entry points in preference to the
    # "scripts" keyword. Entry points provide cross-platform support and allow
    # `pip` to create the appropriate form of executable for the target
    # platform.
    #
    # For example, the following would provide a command called `sample` which
    # executes the function `main` from this package when invoked:
    entry_points={  # Optional
    },

    # List additional URLs that are relevant to your project as a dict.
    #
    # This field corresponds to the "Project-URL" metadata fields:
    # https://packaging.python.org/specifications/core-metadata/#project-url-multiple-use
    #
    # Examples listed include a pattern for specifying where the package tracks
    # issues, where the source is hosted, where to say thanks to the package
    # maintainers, and where to support the project financially. The key is
    # what's used to render the link text on PyPI.
    project_urls={  # Optional
    },
)
