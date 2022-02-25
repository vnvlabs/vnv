import os

VNV_DIR_PATH = os.path.dirname(os.path.realpath(__file__))
VNV_TEMP_PATH = os.path.join(VNV_DIR_PATH,"temp")
if not os.path.exists(VNV_TEMP_PATH):
    os.mkdir(VNV_TEMP_PATH)

VNV_TEMP_TEMP_PATH = os.path.join(VNV_TEMP_PATH,"temp")
if not os.path.exists(VNV_TEMP_TEMP_PATH):
    os.mkdir(VNV_TEMP_TEMP_PATH)


UPDATE_DIR = os.path.join(VNV_TEMP_PATH, "updates")
if not os.path.exists(UPDATE_DIR):
    os.mkdir(UPDATE_DIR)

STATIC_FILES_DIR = os.path.join(VNV_TEMP_PATH, "files")
if not os.path.exists(STATIC_FILES_DIR):
    os.mkdir(STATIC_FILES_DIR)

TEMP_TEMPLATE_DIR = os.path.join(VNV_TEMP_PATH, "templates")
if not os.path.exists(TEMP_TEMPLATE_DIR):
    os.mkdir(TEMP_TEMPLATE_DIR)

RENDERS_DIR = os.path.join(TEMP_TEMPLATE_DIR, "renders")
if not os.path.exists(RENDERS_DIR):
    os.mkdir(RENDERS_DIR)

