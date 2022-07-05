#!/bin/bash

# to generate the .drumkv1 from the .map in mapfiles
# user dont need this script it is for me


ls | grep 'map' > listfolder

while IFS= read -r line; do
i2=`echo "$line" | cut -f 1 -d '.'`
echo "$i2"
drumkv1gen -m ./"$i2".map -o ./"$i2".drumkv1
done < listfolder

rm listfolder

