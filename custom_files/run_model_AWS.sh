#!/bin/bash

#For now, this works by simply copy-pasting these lines into terminal.
#Copy through the comment below, then run the final lines after gcam run
#completes

# User variables, could make these into inputs
GIT_REPO=mmowers/gcam-core #must be publicly accessible repo
GIT_BRANCH=core_v7_run

git clone -b ${GIT_BRANCH} https://github.com/${GIT_REPO}.git gcam
cd gcam/cvs/objects/climate/source
git clone -b gcam-integrationv3 https://github.com/JGCRI/hector.git hector
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
git remote add page_fork https://github.com/pkyle/gcam-core.git
git fetch page_fork gpk/bugfix/gcam7_limitsfix
git merge FETCH_HEAD --no-edit
make xml
cd exe/

GCAM_CONFIG=configuration_ref #This .xml file must be present in exe/
nohup ./gcam.exe -C ${GCAM_CONFIG}.xml & #nohup before and & after to run in background

#RUN FROM TOP THROUGH THIS LINE (monitor run with 'tail -f logs/main_log.txt'), THEN RUN THE FOLLOWING

XMLDB=xmldb_batch #This .xml file must be present in exe/
CLASSPATH=/home/ec2-user/GcamLibraries/jars-6*:/home/ec2-user/ModelInterface-v6/ModelInterface.jar
java -cp ${CLASSPATH} ModelInterface/InterfaceMain -b ${XMLDB}.xml

