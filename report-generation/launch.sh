#!/bin/bash 
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
service mongod start
cd ${SCRIPT_DIR}
virtualenv virt
virt/bin/pip install -r requirements.txt
./run.py $1
