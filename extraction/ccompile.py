import os,sys,json

a = []
if (os.path.exists(sys.argv[1])):
    with open(sys.argv[1],'r') as w:
        a = json.load(w)

a.append({
    "directory" : os.getcwd(),
    "file" : os.path.join(os.getcwd(),sys.argv[-1]),
    "arguments" : sys.argv[2:]
})

with open(sys.argv[1],'w') as w:
    json.dump(a,w, indent=4)



