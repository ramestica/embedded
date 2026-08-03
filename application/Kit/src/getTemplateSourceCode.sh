#
# Create a source code file
#

if [ $# != 2 ]
then
    echo usage: "$FUNCNAME <sh c hc cpp hcpp py> <file-name-with-no-path-nor extension>"
    exit 1
fi

if [[ $2 =~ [\.\:\ ] ]]
then
    echo invalid file name $2
    exit 1
fi
 
case $1 in
    sh )
        temp=$EMBEDDED_ROOT/config/template.sh        
        fn=$2.sh
        ;;
    c )
        temp=$EMBEDDED_ROOT/config/templateCSrc.c
        fn=$2.c
        ;;
    hc )
        temp=$EMBEDDED_ROOT/config/templateCHeader.h
        fn=$2.h
        ;;
    cpp )
        temp=$EMBEDDED_ROOT/config/templateCppSrc.cpp
        fn=$2.cpp
        ;;
    hcpp )
        temp=$EMBEDDED_ROOT/config/templateCppHeader.h
        fn=$2.h
        ;;
    tcpp )
        temp=$EMBEDDED_ROOT/config/templateCppTest.cpp
        fn=$2.cpp
        ;;
    py )
        temp=$EMBEDDED_ROOT/config/templatePyScript.py
        fn=$2.py
        ;;
    * )
        echo invalid file type: $1
        ;;
esac

if [ -f $fn ]
then
    echo file already exists: $fn 
    exit 1
fi

cp $temp $fn

#___oOo___
