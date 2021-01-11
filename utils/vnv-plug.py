import sys, os, json

PLUGIN_FILE = os.environ.get("VNV_PLUGIN_FILE", os.path.join(os.path.expanduser("~"), ".vnv-plugins"))
if os.path.exists(PLUGIN_FILE):
    with open(PLUGIN_FILE, "r") as f:
        master_plugins = json.load(f)
else:
    master_plugins = {"__max-hist__": 25, "__revert__": [], "master": {}}
## What do we want

errors = {

}

## Save the last revision
master_plugins["__revert__"].append(json.loads(json.dumps(master_plugins["master"])))
plugins = master_plugins["master"]


# add: vnv-plug add NAME VERSION DEFAULT FILE
# remove vnv-plug remove NAME VERSION ( version is all then remove all versions
# list vnv-plug list (OPTIONAL NAME) --> list all versions.
def printUsageError(key):
    print("Error running vnv-plug\n")
    print(errors.get(key, key))
    quit()


def writePluginFileAndQuit(message):
    if len(master_plugins["__revert__"]) > master_plugins["__max-hist__"]:
        ind = master_plugins["__max-hist__"]
        master_plugins["__revert__"] = master_plugins["__revert__"][-ind:]

    with open(PLUGIN_FILE, "w") as f:
        json.dump(master_plugins, f)
    print(message)
    quit()


if __name__ == "__main__":

    if len(sys.argv) == 1:
        printUsageError(0)
    elif sys.argv[1] == "add":
        if len(sys.argv) != 6:
            printUsageError(1)
        name = sys.argv[2]
        if (name == "all"):
            printUsageError("Invalid package name")

        version = sys.argv[3]
        default = int(sys.argv[4])
        f = os.path.abspath(sys.argv[5])
        if not os.path.exists(f):
            printUsageError(2)
        if name not in plugins:
            plugins[name] = {version: f, "default": version}
            writePluginFileAndQuit("Added new Package {} (Version: {})".format(name, version))

        else:
            plugins[name][version] = f
            if default:
                plugins[name]["default"] = version
            writePluginFileAndQuit(
                "Added version {} {} to {} ".format(version, "(default)" if default else "", name))
    elif sys.argv[1] == "rm":

        if len(sys.argv) == 3:
            name = sys.argv[2]
            if name == "all":
                plugins = {}
                writePluginFileAndQuit("Removed all plugins")
            elif name in plugins:
                plugins.pop(name)
                writePluginFileAndQuit("Removed all versions of {}".format(name))
            else:
                printUsageError("No package with that name exists")

        elif len(sys.argv) == 4:
            version = sys.argv[3]
            name = sys.argv[2]
            if name not in plugins:
                printUsageError("nameNotInPlugins")
            elif plugins[name]['default'] == version:
                printUsageError("versionIsDefault")
            elif version in plugins[name]:
                plugins[name].pop(version)
                writePluginFileAndQuit("Removed {} version {}".format(name, version))
            else:
                printUsageError("{} Version {} does not exist)".format(name, version))
    elif sys.argv[1] == "revert":
        if len(master_plugins["__revert__"]) > 1:
            # Set plugins to -2 in the revert list (this one + the last one)
            plugins = json.loads(json.dumps(master_plugins["__revert__"][-2]))
            # Pop revert twice:
            master_plugins["__revert__"].pop()
            master_plugins["__revert__"].pop()
            writePluginFileAndQuit("Moved back one step")
        else:
            printUsageError("Nothing to revert to")
    elif sys.argv[1] == "list":
        if len(sys.argv) == 2:
            print(json.dumps(plugins))
        elif sys.argv[2] in plugins:
            print(json.dumps(plugins[sys.argv[2]]))
        else:
            printUsageError("No package with that name")
    elif sys.argv[1] == "hard-reset":
        os.remove(PLUGIN_FILE)
        print("Deleted entire file")
    elif sys.argv[1] == "clear-history":
       master_plugins["__revert__"] = []
       writePluginFileAndQuit("History cleared")
    elif sys.argv[1] == "set-default":
        if len(sys.argv) == 4:
            name = sys.argv[2]
            version = sys.argv[3]
            if name in plugins and version in plugins[name]:
                plugins[name]["default"] = version
                writePluginFileAndQuit("Set default version to {} for {}".format(version,name))
            printUsageError("Invalid name and/or version")
        printUsageError("Not enough args for set-default")
            
    elif sys.argv[1] == "set-max-hist":
       if len(sys.argv) == 3:
            master_plugins["__max-hist__"] = int(sys.argv[2])
            writePluginFileAndQuit("Set history size to {}".format(sys.argv[2]))
            printUsageError("Invalid max history size")
    else:
        print("Unknown Input")


