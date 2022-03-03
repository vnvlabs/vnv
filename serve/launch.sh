#!/bin/bash 

service mongodb start
cd /root/software/vnv/gui/report-generation
./run.py container $1

