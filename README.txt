To compile, the `lib` directory should be made by hand:

   mkdir -p lib/include

Then take the most up to date versions of:
   NamedPipe
   Net2Com
   timer
   he100

and compile their static libs, copying them into lib/

Finally copy their header files into lib/include

The final directory structure should look like this:

lib
   NamedPipe.a
   Net2Com.a
   he100.a
   libhe100.a
   timer.a
   include
      NamedPipe.h
      Net2Com.h
      SC_he100.h
      timer.h

