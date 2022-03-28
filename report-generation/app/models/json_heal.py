import json
import re

import jsonschema
from flask import render_template_string, render_template


class BailException(Exception):
    pass


class Bail:
    def __init__(self, t, val, child=None):
        self.child = child
        self.val = val
        self.type = t


class JsonWithBail:
    escapee = {
        '"': '"',
        '\\': '\\',
        '/': '/',
        "b": '\b',
        "f": '\f',
        "n": '\n',
        "r": '\r',
        "t": '\t'
    }
    NUMS = [str(i) for i in range(0, 10)]

    def __init__(self, text):
        self.text = text
        self.at = 0
        self.ch = " "

    def error(self, m):
        raise SyntaxError(m)

    def next(self, c=None):
        if c and c != self.ch:
            self.error("Expected '" + c + "' instead of '" + self.ch + "'");

        if self.at >= len(self.text):
            raise BailException()

        self.ch = self.text[self.at];
        self.at += 1;
        return self.ch;

    def number(self):
        string = ""
        try:
            number = ""
            self.white()
            if self.ch == '-':
                string = '-';
                self.next();

            while self.ch in JsonWithBail.NUMS:
                string += self.ch;
                self.next();

            if self.ch == '.':
                string += '.';
                while self.next() and self.ch in JsonWithBail.NUMS:
                    string += self.ch;

            if self.ch == 'e' or self.ch == 'E':
                string += self.ch;
                self.next();
                if (self.ch == '-' or self.ch == '+'):
                    string += self.ch;
                    self.next();

                while self.ch in JsonWithBail.NUMS:
                    string += self.ch;
                    self.next();

            try:
                number = float(string)
                return number
            except Exception as e:
                self.error("Bad number");
        except BailException as e:
            return Bail("number", string)

    def string(self):
        stri = ""

        try:
            self.white()
            if self.ch == '"':
                while self.next():
                    if self.ch == '"':
                        self.next();
                        return stri;

                    elif self.ch == '\\':
                        self.next();
                    else:
                        stri += self.ch
            self.error("Bad string");

        except BailException as e:
            return Bail("string", stri)

    def white(self):
        while self.ch and self.ch == ' ':
            self.next()

    def word(self):
        try:
            self.white()
            self.s = self.ch
            if self.ch == "t":
                self.next("t")
                self.next("r")
                self.next("u")
                self.next("e")
                return True
            elif self.ch == "f":
                self.next("f")
                self.next("a")
                self.next("l")
                self.next("s")
                self.next("e")
                return False
            elif self.ch == "n":
                self.next("n")
                self.next("u")
                self.next("l")
                self.next("l")
                return False

            self.error("Unexpected '" + self.ch + "'");
        except BailException as e:
            return Bail("word", self.s)

    def array(self):
        a = []
        try:
            self.white()
            if self.ch == '[':
                self.next('[');
                self.white();
                if self.ch == ']':
                    self.next(']');
                    return a;

                while (self.ch):
                    nn = self.value()

                    if isinstance(nn, Bail):
                        return Bail("array-item", len(a), child=nn)

                    a.append(nn)
                    try:
                        self.white()
                        if self.ch == ']':
                            self.next(']');
                            return a

                        self.next(',');
                        self.white();

                    except Exception as e:
                        return Bail("array-end-or-comma", "")

            self.error("Bad array");
        except BailException as e:
            return Bail("array-items", len(a))

    def object(self):
        key = ""
        ob = {}
        try:
            self.white()
            if self.ch == '{':
                self.next('{');
                self.white();
                if self.ch == '}':
                    try:
                        self.next('}');
                    except Exception as e:
                        pass  # Excepts if end
                    return ob

                while self.ch:
                    key = self.string();
                    if isinstance(key, Bail):
                        key.type = "object-key"
                        return key
                    try:
                        self.white();
                        self.next(':');
                    except BailException as e:
                        return Bail("object-colon", "")

                    if key in ob:
                        raise Exception("Duplicate Key")
                    try:
                        nn = self.value()
                        if isinstance(nn, Bail):
                            return Bail("object-value", key, child=nn)
                        ob[key] = nn
                    except BailException as e:
                        return Bail("object-value", key, child=None)
                    b = False
                    try:
                        self.white();
                        if self.ch == '}':
                            self.next('}');
                            return ob
                        self.white()
                        self.next(',');
                    except:
                        return Bail("object-end-or-comma", "")

            self.error("Bad object");
        except BailException as e:
            return Bail("object-keys", "")

    def value(self):

        self.white();
        if self.ch == "{":
            return self.object()
        elif self.ch == '[':
            return self.array();
        elif self.ch == '\"':
            return self.string();
        elif self.ch == "-" or self.ch in JsonWithBail.NUMS:
            return self.number()
        else:
            return self.word()


def resolve_schema(schema, main_schema):
    while "$ref" in schema:
        a = schema["$ref"].split("/")
        if a[0] != "#":
            raise Exception("Non root refs not supported")
        else:
            schema = main_schema
            for i in a[1:]:
                if isinstance(schema, dict):
                    schema = resolve_schema(schema[i], main_schema)
                elif isinstance(schema, list):
                    schema = resolve_schema(schema[int(i)], main_schema)
    return schema


def getObjectSchema(bail, schema, main_schema):
    if schema.get("type", "object") == "object":
        props = schema.get("properties", {})
        if bail.val in props:
            return props[bail.val]

        pattprops = schema.get("patternProperties", {})
        for i in pattprops:
            if re.match(i, bail.val):
                return pattprops[i]

        additionalProperties = schema.get("additionalProperties", True)
        if isinstance(additionalProperties, dict):
            return additionalProperties

    return {}


def getArraySchema(bail, schema, main_schema):
    if schema.get("type", "array") == "array":
        if "items" in schema:
            items = schema["items"]
            if isinstance(items, dict):
                return items
            elif isinstance(items, list):
                if len(items) > bail.val:
                    return items[bail.val]
                elif "additionalItems" in schema:
                    ad = schema["additionalItems"]
                    if isinstance(ad, dict):
                        return ad


def buildDefault(schema, main_schema):
    schema = resolve_schema(schema, main_schema)
    if "default" in schema:
        return json.dumps(schema["default"])
    elif "oneOf" in schema:
        return buildDefault(schema["oneOf"][0], main_schema)
    elif "const" in schema:
        return str(schema["const"])
    elif schema.get("type", "") == "object":
        d = {}
        props = schema.get("properties", {})
        addProps = schema.get("additionalProperties", True)
        for i in schema.get("required", []):
            if i in props:
                d[i] == buildDefault(props[i], main_schema)
            elif isinstance(addProps, dict):
                d[i] = buildDefault(addProps, main_schema)
            else:
                d[i] = buildDefault({}, main_schema)
        return json.dumps(d)
    elif schema.get("type") == "array":
        return "[]"
    elif schema.get("type") == "number":
        if "min" in schema:
            return schema.get("min", 0)
        elif "max" in schema:
            return schema.get("max")
        return "0"

    elif schema.get("type") == "string":
        e = schema.get("enum", [])
        if len(e):
            return "\"" + e[0] + "\""
        else:
            return "\"\""

    elif schema.get("type") == "boolean":
        return "true"
    elif schema.get("type") == "null":
        return "null"


def oneOfDefaults(schema, main_schema):
    r = []
    for n, i in enumerate(schema.get("oneOf", [])):
        name = i.get("vnv-name", str(n))
        de = buildDefault(i, main_schema)
        desc = i.get("vnv-desc", "")
        r.append([name, de, desc])
    return r


def getSchema(bail, schema, main_schema):
    schema = resolve_schema(schema, main_schema)

    if bail.type in ["object-keys"] or bail.type == "object-key" and len(bail.val) == 0:
        props = schema.get("properties", {})
        return [[
                   i,
                   "\"" + i + "\"",
                   props.get(i, {}).get("description", ""),
                   props.get(i, {}).get("vnvprops", {}),
                   props.get(i, {}).get("vnvparam", {}),
        ] for i in props]

    if bail.type in ["object-key"]:
        props = schema.get("properties", {})
        return [[i,
                 i + "\"",
                 props.get(i, {}).get("description", ""),
                 props.get(i, {}).get("vnvprops", {}),
                 props.get(i, {}).get("vnvparam", {}),
                 ]
               for i in props]
    if bail.type == "object-value":
        if bail.child is not None:
            return getSchema(bail.child, getObjectSchema(bail, schema, main_schema), main_schema)
        else:
            s = getObjectSchema(bail, schema, main_schema)
            d = buildDefault(s, main_schema)
            return [[(d[0:9] + "..." if len(d) > 10 else d), d, "The default value"]]

    if bail.type == "object-colon":
        return [[":", ":", "Colon Seperator"]]
    if bail.type == "object-end-or-comma":
        return [[",", ",", "Add another item"], ["}", "}", "End the object"]]
    if bail.type == "array-item":
        return getSchema(bail.child, getArraySchema(bail, schema, main_schema), main_schema)
    if bail.type == "array-items":
        schema = getArraySchema(bail, schema, main_schema)
        if "oneOf" in schema:
            return oneOfDefaults(schema, main_schema)
        return buildDefault(schema, main_schema)
    if bail.type == "array-end-or-comma":
        return [[",", ",", "Add another item"], ["]", "]", "End the Array"]]
    if bail.type == "string":
        return [[str(i), str(i) + "\"", str(i)] for i in schema.get("enum")]
    if bail.type == "number":
        r = []
        if "min" in schema:
            r.append(["Min", str(schema["min"]), "Minimum Value"])
        if "max" in schema:
            r.append(["Max", str(schema["max"]), "Maximum Value"])
        return r

    if bail.type == "word":
        return [["true", "true", "true"], ["false", "false", "false"], ["null", "null", "null"]]


def autocomplete(txt, schema, row, col, plugins=None):
    #TODO Autocomplete for "plugins"
    try:
        a = txt[0:row + 1]
        a[-1] = a[-1][0:col] + "   "  # add a little white space to catch some issues that might occur
        bail = JsonWithBail("".join(a)).value()
        res = getSchema(bail, schema, schema)

        ret = []
        for i in res:
            if len(i[2]) > 0:
                props = i[3] if len(i) > 3 else {}
                params = i[4] if len(i) > 4 else {}
                desc = render_template("inputfiles/auto_tooltip.html", title=i[0], value=i[1], content=i[2],
                                       hasparams=len(params) > 0, params=params, hasprops=len(props), props=props)
            else:
                desc = ""
            ret.append({"caption": i[0], "value": i[1], "meta": "", "desc": desc})
        return ret

    except Exception as e:
        print(e)


if __name__ == "__main__":
    txt = '{"ghsdgsd": {"sdfsdf" : { "asdaaa" : { "sdf" : "sdfsdf" } } } } '
    schema = '{"type" : "object" , "properties" : { "ghsdgsd" : { "type" : "object" , "properties" : { "sdfsdf" : {"type" : "object"}}}}}'
    process(txt, schema, 1, 22)
