import json
import re


# This is going to suck,..... but
if __name__ == "__main__":
    with open("apex-schema.d.ts","r") as w:
        code = w.read()

    ind =0
    code_no_comments = []
    while ind < len(code):
        if code[ind:].startswith("//"):
            ind = code.find("\n",ind)
        elif code[ind:].startswith("/*"):
            ind += 2
            while not code[ind:].startswith("*/"):
                ind += 1
            ind += 2
        else:
            code_no_comments.append(code[ind])
            ind  += 1
    code = "".join(code_no_comments)

    # now find all the type objects
    types = {}
    ind = 0
    patt = "type\s*(.*)\s* =\s*{"

    a = re.finditer(patt,code)
    for i in a:
        type = i.group(1)
        ind = i.span()[1]
        count = 0
        while ind < len(code):
            if code[ind] == "}":
                if count == 0:
                    types[type] = code[i.span()[1]:ind+1]
                    break
                else:
                    count -= 1
            elif code[ind] == "{":
                count += 1
            ind += 1

    ## Now we have a dictionary of types.
    patt= re.compile("\\s * (.* ?)(\\?)?\\:")
    typemap = {}
    for type in types:
        if type not in typemap:
            typemap[type] = {}
        tstr = types[type]

        a = re.finditer(patt,tstr)

        #Now iterate over vector
        aa = [i for i in a]
        cmap = []
        for n,res in enumerate(aa):
            cmap.append(res.span()[1])
            if n + 1 == len(aa):
                thisStr = tstr[res.span()[1]:].lstrip()
            else:
                thisStr = tstr[res.span()[1]: aa[n+1].span()[0]]

            print(type, res.group(1), thisStr, "\n")



        print("\n")
