#!/bin/sh

autoreconf -i
./configure CC=g++
sudo make install
sudo src/./placonf ttp create "2000-01-01 00:00:00" "2015-01-01 10:10:10" > src/ttp.cert
sudo src/./placonf -C src/ttp.cert id sign 31 31 "2000-02-02 01:01:01" "2015-01-01 00:00:00" > src/user.cert
mv src/user.cert /etc/pla.conf
