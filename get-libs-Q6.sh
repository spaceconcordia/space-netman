NETMAN=$(readlink -f "$0")
NETMAN_DIR=$(dirname "$NETMAN")
mkdir -p bin
mkdir -p lib/include

# Builds libhe100.a 
echo '\n Building HE-100 Library...\n'

cd ../HE100-lib/C
echo "cd: \c" 
pwd
sh mbcc-compile-lib-static-cpp.sh
cp lib/libhe100-mbcc.a $NETMAN_DIR/lib
cp src/SC_he100.h $NETMAN_DIR/lib/include

# Timer library
echo '\n Building timer Library...\n'
cd $NETMAN_DIR
cd ../space-timer-lib 
echo "cd: \c" 
pwd
sh mbcc-compile-lib-static-cpp.sh
cp lib/libtimer-mbcc.a $NETMAN_DIR/lib
cp inc/timer.h $NETMAN_DIR/lib/include

# namedpipe & commander

echo '\n Building Namedpipes and commander...\n'
cd ../space-commander
echo "cd: \c" 
pwd
cp include/Net2Com.h lib/include
cp include/NamedPipe.h lib/include
make buildQ6
cp bin/space-commanderQ6 $NETMAN_DIR/bin
make staticlibsQ6.tar
cp staticlibsQ6.tar $NETMAN_DIR/lib
cd $NETMAN_DIR/lib
tar -xf staticlibsQ6.tar
rm staticlibsQ6.tar

cd $(NETMAN_DIR)/lib
ls | grep 'mbcc*'
