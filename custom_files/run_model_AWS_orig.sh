#!/bin/bash

# Script expects two parameters: the configuration filename and the task number
# Filename should be the base name, not including job number or extension

CONFIGURATION_FILE=${1}_${2}.xml

if [ ! -e $CONFIGURATION_FILE ]; then
	echo "$CONFIGURATION_FILE does not exist; task $2 bailing!"
	exit 
fi

echo "Configuration file: $CONFIGURATION_FILE"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ec2-user/GcamLibraries


# It turns out that to pass data from C++ to Fortran, MiniCAM writes out a 'gas.emk' file
# which is then read in by MAGICC.  This is not good, as multiple instances will stomp
# all over each other.  The long-term solution is to pass internally; for now, we'll 
# create separate exe directories, even though this is a performance hit.

rm -rf exe_$2	 	# just in case
cp -fR exe exe_$2
cd exe_$2

echo "Running GCAM with $CONFIGURATION_FILE..."
# let's keep a copy of config file in the running directory
cp ../$CONFIGURATION_FILE ./config_this.xml
./gcam.exe -C../$CONFIGURATION_FILE > output_${2}.txt 
err=$?

# Clean up
CLASSPATH=/home/ec2-user/GcamLibraries/jars-6*:/home/ec2-user/ModelInterface-v6/ModelInterface.jar

java -cp $CLASSPATH ModelInterface/InterfaceMain -b xmldb_batch_example.xml
java -cp $CLASSPATH ModelInterface/InterfaceMain -b xmldb_batch_example2.xml

#run-diag here

chmod 2775 -R ../exe_$2

if [[ $err -gt 0 ]]; then
	echo "Error code reported: $err"
	echo $err > ../errors/$2
else

         cp gas.emk ../output/gas_${2}.emk




fi

echo "Task $2 is done!"

# return $err