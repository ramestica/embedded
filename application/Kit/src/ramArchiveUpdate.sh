#!/bin/bash

# In the given path look for all lib*.a archives and each one containing the
# given archive name is updated. The new version must exist in the same given
# path.

usage()
{
    echo "usage: " `basename "$0"` " <ar-program-name> <path-to-archives> <archive-name(s)>"
}

updater()
{
    if [ "$1" == "" ]
    then
        return
    fi
    
    local next=
    
    for i in `find $path -maxdepth 1 -name "*.a"`
    do
        for j in `$prgm -t $i`
        do
            for k in $1
            do
                if [ $k == $j ]
                then
                    $prgm -r $i $path/$k
                    if [ $? != 0 ]
                    then
                        echo ERROR "$prgm -r $i $path/$k"
                        exit
                    fi
                    echo in `basename $i` updating $k 
                    next=$(echo $next $(basename $i))
                fi
            done
        done
    done

    #
    # remove repeated names; repeated names might happen when many archives
    # depend on the same one that needs update and the above loop does not take
    # care of that.
    #
    updater $(printf '%s\n' $next | sort -u)
}

if [ "$#" -lt "3" ]
then
    usage
    exit 1
fi

prgm=$1
path=$2
shift 2

updater $*

#___oOo___
