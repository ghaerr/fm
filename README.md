# Tiny file manager for UNIX and ELKS

This project is forked from `noice` (https://git.2f30.org/noice/), a small file browser.
Thanks to lostd and sin at `2f30.org` for a well-written program with which to start from.

The goal of this project is to produce a very small file manager/browser
that can run identically on UNIX ANSI terminals, as well as the ELKS 16-bit
Linux operating system.

As such, the `ncurses` library dependency is removed, as all terminal
sequences are output using ANSI x3.64 standard, including color output
sequences as well as arrow key and mouse wheel input parsing, using
a small set of included routines.

This is a work in progress.

# How do I compile/run it?

For UNIX, type `make`.

For ELKS, type `make -f Makefile.elks`.

The resulting output executable is `fm`.

# What keys are used to operate the program?

The file [HOWTO](https://github.com/ghaerr/fm/blob/master/HOWTO) describes
the keystrokes to browse a filesystem.

For more information, refer to the man page.

## Screenshots

`fm` file manager running on macOS
![ss1](https://github.com/ghaerr/fm/blob/master/Screenshots/FM_terminal.png)
