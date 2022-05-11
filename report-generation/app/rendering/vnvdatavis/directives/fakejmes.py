import json

from jmespath.visitor import Options, TreeInterpreter
from jmespath import parser, functions
from jmespath.functions import TYPES_MAP, REVERSE_TYPES_MAP, Functions

VnVMap = ['IBoolNode',
          'ICommInfoNode',
          'ICommMap',
          'IDataNode',
          'IDoubleNode',
          'IFloatNode',
          'IInfoNode',
          'IInjectionPointNode',
          'IIntegerNode',
          'IJsonNode',
          'ILogNode',
          'ILongNode',
          'IMapNode',
          'IRootNode',
          'IShapeNode',
          'IStringNode',
          'ITestNode',
          'IUnitTestNode',
          'IUnitTestResultNode',
          'IUnitTestResultsNode'
          ]

VnVArray = ["IArrayNode"]

TYPES_MAP.update({f'VnVReader.{a}': "object" for a in VnVMap})
TYPES_MAP.update({f'VnVReader.{a}': "array" for a in VnVArray})

REVERSE_TYPES_MAP["object"] = tuple(VnVMap + ['dict', 'OrderedDict'])
REVERSE_TYPES_MAP["array"] = tuple(VnVArray + ['list', "_Projection"])


def _equals(x, y):
    if _is_special_integer_case(x, y):
        return False
    else:
        return x == y


def _is_special_integer_case(x, y):
    if isinstance(x, int) and (x == 0 or x == 1):
        return y is True or y is False
    elif isinstance(y, int) and (y == 0 or y == 1):
        return x is True or x is False


TreeInterpreter.COMPARATOR_FUNC.update({
    'eq': _equals,
    'ne': lambda x, y: not _equals(x, y),
})


class RootInterpreter(TreeInterpreter):

    def isDefactoList(self, element):
        return isinstance(
            element,
            list) or (
                       hasattr(
                           element,
                           "__getType__") and element.__getType__() == "array")

    def __init__(self, dict_class=None, custom_functions=None):
        super(
            RootInterpreter, self).__init__(
            Options(dict_cls=dict_class, custom_functions=custom_functions))

    def visit_field(self, node, value):
        try:
            return value[node['value']]
        except BaseException:
            return None

    def visit_filter_projection(self, node, value):
        base = self.visit(node['children'][0], value)
        if not self.isDefactoList(base):
            return None
        comparator_node = node['children'][2]
        collected = []
        for element in base:
            if self._is_true(self.visit(comparator_node, element)):
                current = self.visit(node['children'][1], element)
                if current is not None:
                    collected.append(current)
        return collected

    def visit_flatten(self, node, value):
        base = self.visit(node['children'][0], value)
        if not self.isDefactoList(base):
            # Can't flatten the object if it's not a list.
            return None
        merged_list = []
        for element in base:
            if self.isDefactoList(element):
                merged_list.extend(element)
            else:
                merged_list.append(element)
        return merged_list

    def visit_index(self, node, value):
        # Even though we can index strings, we don't
        # want to support that.

        if self.isDefactoList(value):
            try:
                return value[node['value']]
            except IndexError as e:
                pass
        return None

    def visit_slice(self, node, value):
        if not self.isDefactoList(value):
            return None
        s = slice(*node['children'])
        return value[s]

    def visit_not_expression(self, node, value):
        original_result = self.visit(node['children'][0], value)
        if isinstance(original_result, int) and original_result == 0:
            # Special case for 0, !0 should be false, not true.
            # 0 is not a special cased integer in jmespath.
            return False
        return not original_result

    def visit_projection(self, node, value):
        base = self.visit(node['children'][0], value)
        if not self.isDefactoList(base):
            return None
        collected = []
        for element in base:
            current = self.visit(node['children'][1], element)
            if current is not None:
                collected.append(current)
        return collected

    def _is_false(self, value):
        # This looks weird, but we're explicitly using equality checks
        # because the truth/false values are different between
        # python and jmespath.
        if (hasattr(value, "__getType__")):
            return len(value) == 0

        return (value == '' or value == [] or value == {} or value is None or
                value is False)


class VnVJsonEncoder(json.JSONEncoder):
    def default(self, o):
        if hasattr(o, "__json__"):
            return o.__json__();
        return json.JSONEncoder.default(self, o)


class CustomVnVFunctions(functions.Functions):

    @functions.signature({"types": []})
    def _func_as_json(self, s):
        # Given a object return it as json encoded string
        return json.dumps(s, cls=VnVJsonEncoder)

    @functions.signature({"types": []}, {"types": []})
    def _func_vnv_join(self, sep, obj):
        if hasattr(obj, "__getType__") and obj.__getType__() == "array":
            ss = ""
            for i in obj:
                ss += str(i)
            return ss
        return sep.join(obj)


    ### TODO This wont work because we have not defined the (*,+) operator for
    ### the DataBase class.
    @functions.signature({"types": []},{"types": []}, {"types": []})
    def _func_axpy(self, alpha, sep, obj):
        return alpha * sep + obj



class VnVExpression:
    def __init__(self, parse):
        self.parse = parse

    def search(self, value):
        interpreter = RootInterpreter(custom_functions=CustomVnVFunctions())
        return interpreter.visit(self.parse.parsed, value)


def compile(expression):
    return VnVExpression(parser.Parser().parse(expression))
