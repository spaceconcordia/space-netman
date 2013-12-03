mkdir -p bin
echo 'Building HE-100 Library...'
make he100PC
echo 'Building timer library...'
make timerPC
echo 'Building pipes library...'
make namedpipePC
echo 'Building commander...'
make commanderPC
cd lib
ls -al
