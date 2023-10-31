#!/bin/bash
echo "*****Creating a symlink in parent directory*****"
ln -s `pwd` vargrind
mv vargrind ..
echo "*****Making sure Valgrind is built in parent directory*****"
./build-valgrind.sh -a
echo "*****Patching valgrind/Makefile.am and making a backup file*****"
patch -b ../Makefile.am < Patches/patch.Makefile.am
echo "*****Patching valgrind/configure.ac and making a backup file*****"
sed -i '/lackey\/tests\/Makefile/a \   vargrind/Makefile' ../configure.ac
#echo "*****Patching valgrind/tests/check_headers_and_includes and making a backup file*****"
#patch -b ../tests/check_headers_and_includes < Patches/check_headers_and_includes.patch
echo "*****Rebuilding valgrind with the patched files...*****"
./build-valgrind.sh -a
echo "*****Installed vargrind, testing with Date program...*****"
../inst/bin/valgrind --tool=vargrind date
echo "Vargrind Setup complete!"
