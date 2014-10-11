#!/bin/sh
set -e

distdir=libetpan-travis-build
./autogen.sh --with-curl=no --disable-db --with-expat=no
make dist distdir=$distdir
tar xzf $distdir.tar.gz
cd $distdir
./configure --with-curl=no --disable-db --with-expat=no
make
cd tests
make imap-sample
