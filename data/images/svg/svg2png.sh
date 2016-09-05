#! /usr/bin/env sh

set -e
set -u

OUT_DIR="$(dirname "$0")/../"
INKSCAPE="$(which inkscape)"

err() {
   echo "*** $@" 1>&2
}

inkscape_export() {
   file="$1"
   out="$OUT_DIR/$(echo "$file" | cut -d '.' -f 1)"

   if [ x"$file" = x"war2edit.svg" ]; then
      WIDTH=512
      HEIGHT=512
   else
      WIDTH=100
      HEIGHT=100
   fi
   "$INKSCAPE" \
      --export-width="$WIDTH" \
      --export-height="$HEIGHT" \
      --export-png="$out".png \
      "$file"
}

FILES="$(ls -1 *.svg)"

for f in $FILES; do
   inkscape_export "$f"
done
