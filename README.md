# xtc-length

This program reads the number of frames in a Gromacs XTC file, the number of atoms and the simulation time in nanoseconds.  It reads only the header from each frame so is faster than the Gromacs utility GMX CHECK, but does not check for errors in the file.

## To Compile
From the directory where the project was downloaded:

`cmake .; make install`

If you with to install to a directory other than default use `cmake . -DCMAKE_INSTALL_PREFIX=<directory>`

Since there is only one source file it is also possible to call the compiler directly if you prefer:

`gcc -o xtc-length xtc-length.c`
