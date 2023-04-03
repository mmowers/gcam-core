#!/bin/bash

#Run this from the terminal with 'nohup ./run_model_AWS.sh &'
#After running, you can follow the output with 'tail -f nohup.out'

# User variables, could make these into inputs
GIT_REPO=mmowers/gcam-core #must be publicly accessible repo
GIT_BRANCH=core_run

git clone -b ${GIT_BRANCH} https://github.com/${GIT_REPO}.git gcam
cd gcam/cvs/objects/climate/source
git clone -b gcam-integration https://github.com/JGCRI/hector.git hector
cd /home/ec2-user/gcam
scl enable gcc-toolset-9 bash
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.352.b08-2.el8_7.x86_64
export JAVA_INCLUDE=${JAVA_HOME}/include
export JAVA_LIB=${JAVA_HOME}/jre/lib/amd64/server
export XERCES_INCLUDE=/home/ec2-user/GcamLibraries/xerces_3.2.2_DT2/include
export XERCES_LIB=/home/ec2-user/GcamLibraries/xerces_3.2.2_DT2/lib
export JARS_LIB=/home/ec2-user/GcamLibraries/jars-6/*
export EIGEN_INCLUDE=/home/ec2-user/GcamLibraries/eigen
export TBB_INCLUDE=/home/ec2-user/GcamLibraries/tbb/include
export TBB_LIB=/home/ec2-user/GcamLibraries/tbb/lib
export BOOST_LIB=/home/ec2-user/GcamLibraries/boost_1_77_zt1/lib
export BOOST_INCLUDE=/home/ec2-user/GcamLibraries/boost_1_77_zt1/include
make gcam -j 12
make xml
cd exe/

GCAM_CONFIG=configuration_ref #This .xml file must be present in exe/
./gcam.exe -C ${GCAM_CONFIG}.xml

XMLDB=xmldb_batch #This .xml file must be present in exe/
CLASSPATH=/home/ec2-user/GcamLibraries/jars-6*:/home/ec2-user/ModelInterface-v6/ModelInterface.jar
java -cp ${CLASSPATH} ModelInterface/InterfaceMain -b ${XMLDB}.xml

