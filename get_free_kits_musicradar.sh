#!/bin/bash
#
# Small downloader script that grabs some freely available drum kit sample
# libraries, unpacks them and copies the corresponding drumkv1gen .map
# and .drumkv1 files into place for "Plug&Play" use.

# will store libraries in a subfolder "FreeKits"; does it exist already?
if [ -d FreeKitsMusicradar ]; then
  echo "Folder 'FreeKitsMusicradar' already exists; won't touch it, exiting."
  exit 1
fi

# Else, go ahead
mkdir FreeKitsMusicradar

cd FreeKitsMusicradar

echo "downloading"
wget -q https://cdn.mos.musicradar.com/audio/samples/musicradar-drum-samples.zip

echo "unpacking"
unzip -qq musicradar-drum-samples.zip

echo "Now you have some errors messages because I havent done all the kits but it works for some of them !"

# no need to keep the .zip
rm musicradar-drum-samples.zip

cd "musicradar-drum-samples/Drum Kits/"

ls > ../listfolder

while IFS= read -r line; do

cd "$line" 
cp "../../../../mapfiles/musicradar/Drum Kits/$line.map" .
cp "../../../../mapfiles/musicradar/Drum Kits/$line.drumkv1" .
cd ..
done < ../listfolder

rm ../listfolder


# Now reorganize Assorted Kits
echo "reorganize Assorted Kits"

# Cymbals
cd "../Assorted Hits/Cymbals"
cp './Trash Crashes/'* ./
rm -r './Trash Crashes/'
cp "../../../../mapfiles/musicradar/Assorted Hits/Cymbals.drumkv1" .

# Hi Hats
cd "../Hi Hats"
cp './Acoustic/'* ./
cp './Sabian B8/'* ./
cp './Zildjian K Hats/'* ./
rm -r './Acoustic/' './Sabian B8/' './Zildjian K Hats/'
cp "../../../../mapfiles/musicradar/Assorted Hits/Hi Hats.drumkv1" .

# Kick
cd "../Kicks"
cp './Acoustic/'* ./
cp './Kes Kick/'* ./
cp './Loose Kick/'* ./
rm -r './Acoustic/' './Kes Kick/' './Loose Kick/'
cp "../../../../mapfiles/musicradar/Assorted Hits/Kicks.drumkv1" .

# snares
cd "../Snares"
ls > snares.txt
while IFS= read -r line; do
 cp "./$line/"* ./
 rm -r "./$line/"
done < snares.txt
rm snares.txt

cp "../../../../mapfiles/musicradar/Assorted Hits/Snares1.drumkv1" .
cp "../../../../mapfiles/musicradar/Assorted Hits/Snares2.drumkv1" .


echo "Done. Find the ready-to-use kits in folder 'FreeKitsMusicradar'."
echo "Please note that only a subset of the samples of each kit is mapped"
echo "The ones I didn't know where to assign are at the beginning (note 0,1,2...)"
echo "For folder /Assorted Hits There is 1 .drumkv1 file per instrument with no mapping (just the .wav in the alphabetic order from note 0"
echo "Visit the homepages of these projects for more info:"
echo "https://www.musicradar.com/news/drums/1000-free-drum-samples"


