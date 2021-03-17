#!/bin/bash
num_bytes=$(($2 - 1))
filename=$1

pwgen -N1 $num_bytes > $filename
