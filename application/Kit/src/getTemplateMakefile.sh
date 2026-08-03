#
# Create a Makefile
#

if [ -f Makefile ]
then
    echo file already exists: Makefile 
    exit 1
fi

cp $EMBEDDED_ROOT/config/templateMakefile Makefile

#___oOo___
