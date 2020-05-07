.. Example Project documentation master file, created by
   sphinx-quickstart on Mon Apr 27 14:52:36 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Example Project's documentation!
===========================================

.. vnv-chart:: 
   {
    "type" : "line",
    "data" : {
        "labels" : ["January", "February", "March", "April", "May", "June", "July"],
        "datasets" : [{
            "label": "My First dataset",
            "backgroundColor": "rgb(255, 99, 132)",
            "borderColor": "rgb(255, 99, 132)",
            "data": [0, 10, 5, 2, 20, 30, 45]
        }]
    }
    }

.. vnv-paraview:: https://data.kitware.com/api/v1/item/59de9d418d777f31ac641dbe/download


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   vnvtree/index

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
