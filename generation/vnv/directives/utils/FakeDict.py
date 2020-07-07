
from collections.abc import MutableMapping

from docutils.parsers.rst import directives


class FakeDict(MutableMapping):
    """A dictionary that calls a function when a requested key does not
    exist in the dictionary"""

    def __init__(self, func, *args, **kwargs):
        self.func = func
        self.store = dict()
        self.update(dict(*args, **kwargs))  # use the free update to set keys

    def __getitem__(self, key):
        if key in self.store:
            return self.store[key]
        return self.func(key)

    def __setitem__(self, key, value):
        self.store[key] = value

    def __delitem__(self, key):
        del self.store[key]

    def __iter__(self):
        return iter(self.store)

    def __len__(self):
        return len(self.store)

