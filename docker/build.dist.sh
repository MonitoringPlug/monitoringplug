#!/bin/bash

dnf -y upgrade
dnf -y install @buildsys-build dnf-plugins-core git \
               autoconf automake libtool \
               libxslt docbook-style-xsl

cd /src

./autogen.sh
./configure
make dist-gzip

chown --recursive --reference=/src /src
