from vnv import VnVReader
import json
from basic_tree import VnVTreeGenerator


VnVReader.Initialize([],"./vv-input.json")

reader = VnVReader.ReaderWrapper("basic","dummy",json.dumps({}))

rootNode = reader.get()

tree = VnVTreeGenerator.VnVTreeGenerator()

print(tree.generateConfigFile(rootNode))

tree.config(quickstart={}, build=True, build_opts=["html"])
tree.generateTree("./test-output", rootNode)


VnVReader.Finalize()

