import pymongo


def list_mongo_collections():
    a = pymongo_database.list_collection_names()
    return a


def collection_exists(name):
    return name in list_mongo_collections()


try:
    MONGO_URL = 'mongodb://localhost:27017/'
    MONGO_DB = "vnv"
    pymongo_client = pymongo.MongoClient(MONGO_URL)
    pymongo_database = pymongo_client.get_database(MONGO_DB)

    if not collection_exists("__vnv_reader_input_files__"):
        pymongo_database.create_collection("__vnv_reader_input_files__")
    input_collection = pymongo_database.get_collection("__vnv_reader_input_files__")
    MONGO_AVAILABLE = True
except:
    MONGO_AVAILABLE = False
    print("MONGO NOT RUNNING -- CANNOT PERSIST TO DB")


def Configured():
    return MONGO_AVAILABLE


def validate_name(name):
    if Configured():
        i = 0
        while collection_exists(f'{name}_{i}'):
            i = i + 1
        return f'{name}_{i}'
    return name


def loadInputFile(name):
    if Configured():
        return input_collection.find_one({"name" : name})
    return None

def persistInputFile(file):
    if Configured():
        input_collection.update_one({"name": file.name}, {"$set": file.toJson()}, True)

