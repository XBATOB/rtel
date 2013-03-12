#!/bin/sh
# $Id: extract-messages.sh 2199 2010-06-22 11:08:01Z xbatob $

BASEDIR="../src/"    # root of translatable sources
PROJECT="rtel"       # project name
BUGADDR="s.xbatob@gmail.com"    # MSGID-Bugs
WDIR=`pwd`           # working dir

echo "Extracting messages"
cd ${BASEDIR}
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' | sort > ${WDIR}/infiles.list
cd ${WDIR}
xgettext --from-code=UTF-8 -k_ -kN_ \
    --msgid-bugs-address="${BUGADDR}" \
    --files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${PROJECT}.pot \
    || { echo "error while calling xgettext. aborting."; exit 1; }
echo "Done extracting messages"

echo "Merging translations"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
  echo $cat
  msgmerge -o $cat.new $cat ${PROJECT}.pot
  mv $cat.new $cat
done
echo "Done merging translations"

echo "Cleaning up"
cd ${WDIR}
rm -f rcfiles.list infiles.list rc.cpp
echo "Done"
