#!/bin/bash

# If Valgrind is installed for the first time (or it is not added into PATH)
# user must either logout either execute source .bashrc

set -e
set -u
set -o pipefail

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

if [[ $# -eq 0 ]]
then 
    set -- "-h"
fi

while getopts 'apbtTh' OPTION; do
    case "$OPTION" in
        a)           
            #all
            pushd ../ 
            ./autogen.sh && ./configure --prefix=`pwd`/inst
            popd
            ;&
        b)
            # build
            pushd ../ 
            make && make install
            popd
            ;&
        p)
            #path
            if ! grep -q valgrind ~/.bashrc ; then 
                pushd ../
                echo '# Export PATH for Valgrind' >> ~/.bashrc
                echo 'export PATH='$PATH:`pwd`/inst/bin >> ~/.bashrc
                popd
            fi

            if ! grep -q VALGRIND_LIB ~/.bashrc ; then 
                pushd ../
                echo 'export VALGRIND_LIB='`pwd`/.in_place >> ~/.bashrc
                popd
            fi
            ;;
	t)
		pushd tests/unit/
		make all
		echo "Tests built"
		popd
		;;
	T)	testFile=tests/bin/test_all
		if [ ! -f $testFile ] ; then
			echo "Tests not built, building..."
			./build-valgrind.sh -t;
		fi
		
		$testFile
		;;
	h | ?)
            echo "-a"
            echo "   build valgrind"
            echo
            echo "-b"
            echo "   build project and check path"
            echo
            echo "-p"
            echo "   check path and upgrade if required"
            echo
	    echo "-t"
	    echo "   build all tests"
	    echo
	    echo "-T"
	    echo "   run all tests"
	    echo
            echo "-h"
            echo "   show this menu"
            ;;
    esac
done
shift "$(($OPTIND -1))"
