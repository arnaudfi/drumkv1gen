#!/bin/bash

# to generate the .drumkv1 from the .map in mapfiles
# user dont need this script it is for me


liste=`ls ./mapfiles`
echo $liste

for i in $liste 
do
i2=`echo "$i" | cut -f 1 -d '.'`
drumkv1gen -m ./mapfiles/$i2.map -o ./mapfiles/$i2.drumkv1
done
