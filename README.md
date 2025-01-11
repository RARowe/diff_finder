# PNG Diff finder

This is really bad code that's heavily modified from the libspng examples.
I did just enough to get it working but didn't clean it up.
Both input pngs need to be exactly the same size.

The out.png file will show the difference between the two files.

# Running
Install libspng the old fashioned way or `sudo apt install libspng-dev`.

Compile and run with `gcc main.c -lspng && ./a.out`.

If you use a custom install, you'll need to set you `-I` and `-L` flags to point at the right directories.
