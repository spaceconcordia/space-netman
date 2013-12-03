mkdir -p bin
mkdir -p lib/include
echo 'Building HE-100 Library...\n'
make he100Q6
echo 'Building timer library...\n'
make timerQ6
echo 'Building pipes library...\n'
make namedpipeQ6
echo 'Building commander...\n'
make commanderQ6
cd lib
ls -al
