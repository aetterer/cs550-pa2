#!/bin/bash
ip=$(ifconfig | grep 192.168 | awk '{print $2}')
fuser -k $(sed -n 1p configs/peernode${1}.cfg)/tcp
./bin/peer_node configs/peernode${1}.cfg $ip 20769
