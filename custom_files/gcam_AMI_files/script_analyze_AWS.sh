#!/bin/bash 

CLASSPATH=/home/ec2-user/GcamLibraries/jars-6*:/home/ec2-user/ModelInterface-v6/ModelInterface.jar

cd /home/ec2-user/gcam6/exe

java -cp $CLASSPATH ModelInterface/InterfaceMain -b xmldb_batch_example.xml
java -cp $CLASSPATH ModelInterface/InterfaceMain -b xmldb_batch_example2.xml
