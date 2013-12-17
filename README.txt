To compile, the `lib` directory should be made by hand:

   mkdir -p lib/include

Then take the most up to date versions of:
   NamedPipe
   Net2Com
   timer
   he100

Run the scripts (Q6 or PC) for all static libraries needed:
sh get-libs-Q6.sh
sh get-libs-PC.sh

The final directory structure should look like this:

lib
   NamedPipe.a
   NamedPipe-mbcc.a
   Net2Com.a
   Net2Com-mbcc.a
   libhe100.a
   libhe100-mbcc.a
   timer.a
   timer-mbcc.a
   include
      NamedPipe.h
      Net2Com.h
      SC_he100.h
      timer.h

To compile the netman for Q6 or PC respectively:
make Q6
make all

The bin folder should like this:

bin
  space-commander
  space-commanderQ6
  sat
  sat-mbcc
  gnd
  gnd-mbcc

Example Use:

=====================================================================
Simulation of ground station with satellite netman & commander on PC:
=====================================================================

Open 4 terminals:

run
  ./sat
  ./gnd
  ./space-commander

Type in last terminal:

Command Step 1
echo -n -e \\x31 > gnd-input

Command Step 2
echo -n -e \\x21 > gnd-input 

======
On Q6:
======

Copy sat-mbcc to Q6
run
  ./sat-mbcc

Type in HEX symbols on keyboard on ground station computer with the Kenwood radio terminal:

Example of input on the Kenwood Putty terminal:

010003c53f2a5a

The satellite netman will receive the frame in ascii, and convert to hex.
The frame will be converted on the netman as: 
0x01 0x00 0x03 0xc5 0x3f 0x2a 0x5a






