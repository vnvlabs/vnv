import pymongo


def Configured():
    return MONGO_AVAILABLE

def list_mongo_collections():
    if Configured():
        a = pymongo_database.list_collection_names()
        return a
    return []

def collection_exists(name):
    return name in list_mongo_collections()


try:
    MONGO_URL = 'mongodb://localhost:27017/'
    MONGO_DB = "vnv"
    pymongo_client = pymongo.MongoClient(MONGO_URL, serverSelectionTimeoutMS = 2000)
    pymongo_database = pymongo_client.get_database(MONGO_DB)
    MONGO_AVAILABLE = True

    if not collection_exists("__vnv_reader_input_files__"):
        pymongo_database.create_collection("__vnv_reader_input_files__")
    input_collection = pymongo_database.get_collection("__vnv_reader_input_files__")
except Exception as e:
    MONGO_AVAILABLE = False
    print("MONGO NOT RUNNING -- CANNOT PERSIST TO DB")




def validate_name(name):
    if Configured():
        if not collection_exists(name):
            return name

        i = 0
        while collection_exists(f'{name}_{i}'):
            i = i + 1
        return f'{name}_{i}'
    return name



def list_all_files():
    if Configured():
        a = pymongo_database.list_collection_names()
        a.remove("__vnv_reader_input_files__")
        return a
    return []

def loadInputFile(name):
    if Configured():
        return input_collection.find_one({"name" : name})
    return None

def persistInputFile(file):
    if Configured():
        input_collection.update_one({"name": file.name}, {"$set": file.toJson()}, True)


def list_input_files():
    if Configured():
        return input_collection.find()
    return []


def deleteInputFile(name):
    input_collection.delete_one({"name": name})


def removeFile(name):
    pymongo_database.drop_collection(name)

