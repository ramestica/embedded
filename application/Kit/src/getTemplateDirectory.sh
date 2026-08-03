#
# Create a root directory structure.
#

if [ $# != 2 ]
then
    echo usage: "$FUNCNAME <module or root> <destination>"
    exit 1
fi

if [ -d $2 ]
then
    echo destination already exists $2
    exit 1
fi

case $1 in
    module )
        for i in include src test
        do
            mkdir -p $2/$i
        done
        tmp=$PWD
        cd $2/src
        getTemplateMakefile
        cd $tmp
        cd $2/test
        getTemplateMakefile
        cd $tmp
        ;;
    root )
        for i in bin lib include
        do
            mkdir -p $2/$i
        done
        ;;
    * )
        echo invalid directory type: $1
        ;;
esac
 
#___oOo___
