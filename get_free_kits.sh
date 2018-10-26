#!/bin/bash
#
# Small downloader script that grabs some freely available drum kit sample
# libraries, unpacks them and copies the corresponding drumkv1gen .map
# and .drumkv1 files into place for "Plug&Play" use.

LISTFILE="freekits.txt"

# will store libraries in a subfolder "FreeKits"; does it exist already?
if [ -d FreeKits ]; then
  echo "Folder 'FreeKits' already exists; won't touch it, exiting."
  exit 1
fi

# Else, go ahead
mkdir FreeKits

while IFS= read listentry
do
  args=($(echo $listentry))

  folder=${args[0]}
  base_uri=${args[1]}
  filename=${args[2]}
#  echo Folder: ${folder}
#  echo Base URI: ${base_uri}
#  echo Filename: ${filename}

  echo "Processing kit ${folder} from URI ${base_uri}..."
  cd FreeKits
  mkdir ${folder}
  cd ${folder}
  echo "  Downloading.."
  fullname=${base_uri}/${filename}
  wget -q ${fullname}
  echo "  Unpacking.."
  mkdir samples
  cd samples
  unzip -qq ../${filename}
  cd ..
  echo "  Copying map/drumkv1 files into place.."
  cp ../../mapfiles/${folder}.map .
  cp ../../mapfiles/${folder}.drumkv1 .
  cd ../..
done < "$LISTFILE"

echo "Done. Find the ready-to-use kits in folder 'FreeKits'."
echo "Please note that only a subset of the samples of each kit is actually"
echo "contained in the provided mapping. There are more that you could"
echo "(and perhaps should) map yourself to your liking."
echo
echo "Visit the homepages of these projects for more info:"
echo "  http://machines.hyperreal.org"
echo "  http://little-scale.blogspot.com/"



