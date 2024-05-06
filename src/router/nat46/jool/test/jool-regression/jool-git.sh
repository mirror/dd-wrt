#!/bin/bash
# This updates the Jool repository, then checks out whichever branch you want to test

# Load configuration 
. config

echo "Cleaning Jool repository"|tee -a $RESULT_LOG
# Move to Jool directory and clean mod and usr
cd $JOOL_DIR/mod
make clean
cd $JOOL_DIR/usr
make clean
cd $JOOL_DIR

# Make sure we have nothing polluting the directory
git checkout .

echo "Jool repository clean"|tee -a $RESULT_LOG
echo "Updating Jool repository"|tee -a $RESULT_LOG

# Update
git pull

echo "Checking out "|tee -a $RESULT_LOG
# Checkout whichever branch we want to test
git checkout $BRANCH

