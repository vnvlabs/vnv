#!/bin/bash 


#The users volume is mounted at /container. 
#We need to configure mongo to write all its data there so it is 
#persisted in the volume (instead of the container, which might be destroyed.)
MONGO_DIR=/container/mongo
if [ ! -d "${MONGO_DIR}" ]; then

#Stop MONGO (it shouldnt be running anyway but just in case. )
service mongodb stop

# Create the mongo dir inside the volume
mkdir -p ${MONGO_DIR}
mkdir -p ${MONGO_DIR}/db

# Write a readme file 
cat << EOF > ${MONGO_DIR}/README
"This directory holds your guis database. Its your container, so 
do what you like, but, if you delete this directory or any of the files 
in it then I have no idea what will happen." > /home/$2/.mongo_do_not_delete/README
EOF

# Update the mongo configuration to point to the correct file. 
cat << EOF > /etc/mongodb.conf

dbpath= ${MONGO_DIR}/db 

logpath=${MONGO_DIR}/mongodb.log

logappend=true

bind_ip = 127.0.0.1

journal=true
EOF


# Start mongo.sh
service mongodb start

#Start the GUI
cd /root/software/vnv/gui/report-generation 
./run.py container $1


