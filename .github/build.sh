#!/bin/bash

sudo apt-get install -y git sdcc cmake


# this repository
git clone https://github.com/mightymos/RF-Bridge-OB38S003.git
cd RF-Bridge-OB38S003/
echo "Building firmware..."
make
