mkdir -p bin
mkdir -p lib/include
echo 'Building HE-100 Library...\n'
make he100PC
echo 'Building timer library...\n'
make timerPC
echo 'Building pipes library...\n'
make namedpipePC
echo 'Building commander...\n'
make commanderPC
cd lib
ls -al