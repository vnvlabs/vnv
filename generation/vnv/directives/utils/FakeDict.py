from collections.abc import MutableMapping
import re
import json as jsonLoader
from . import JmesSearch

# TODO Could precompile the jmes to check its valid (from a lex standpoint)
# TODO in advance.


class JmesTerm:
    def __init__(self, mess):
        self.content = mess


def checkJmes(mess):
    return JmesTerm(mess)


def jmesChecker(key):
    return checkJmes


class FakeDict(MutableMapping):
    """A dictionary that calls a function when a requested key does not
    exist in the dictionary"""

    def __init__(self, *args, **kwargs):
        self.func = jmesChecker
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


def _formatMessage(message, args, formatter, def_val):
    res = '\\$\\$([a-zA-Z0-9_]*)\\$\\$'

    xx = re.sub(
        res,
        lambda x: formatter(
            args.get(
                x.group(1),
                def_val)),
        message)
    return xx


def process(
        options,
        node,
        content,
        formatter=lambda x: jsonLoader.dumps(x),
        def_val=[]):
    # Process the content for any vnv directives.
    subs = {}
    for sub in options.keys():
        v = options[sub]
        if isinstance(v, JmesTerm):
            subs[sub] = JmesSearch.getJMESNode(node, v.content)
    return _formatMessage(content, subs, formatter, def_val)
