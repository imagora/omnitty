#!/bin/sh

if ! [ -f configure ]; then
   echo "./configure does not exist. Run autoconf."
   exit 1
fi

/bin/rm -rf autom4te.cache

X=`tempfile`
/bin/rm -f $X
mkdir $X

BASENAME=rote-`grep ^AC_INIT configure.ac | cut -d, -f2 | cut -d' ' -f2 | \
                        cut -d')' -f1 `

(cd .. && cp -a rote $X/$BASENAME)
(cd $X && find $X -name CVS -type d -exec rm -rf {} \; 2>/dev/null )

(cd $X && tar -zc $BASENAME) > ../$BASENAME.tar.gz

/bin/rm -rf $X
echo "../$BASENAME.tar.gz generated."

