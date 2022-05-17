#!/bin/bash 

# Start mongo.sh
service mongodb start

# Start Theia in the source directory. 
cd /theia 
node /theia/src-gen/backend/main.js ${SOURCE_DIR} --hostname=0.0.0.0 &

# Start the paraview visualizer server
cd /paraview 
$PVPYTHON -m paraview.apps.visualizer --host 0.0.0.0 --data / --port 9000 --timeout 600000 & 

#Start the GUI
cd /software/vnv/gui/report-generation 
./run.py container $1


