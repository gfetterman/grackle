# Grackle

`Grackle` is a command-line tree-walking interpreter for a limited subset of the
[Racket](https://racket-lang.org) dialect of Lisp.

This is a hobby project, with features being added as I figure them out, and 
have time.

## Supported language features

* arbitrarily-nested s-expression arithmetic (on fixed-precision integers)
* variable definition
* list manipulation
* predicate and conditional expressions
* user-defined lambdas and named functions, with lexical scoping
* boolean, integer, string, list, and function types

A fuller list may be found [here](language_features.md).

## Next language feature to be added

* floating-point arithmetic

## Installation

Binaries are not currently available. Compilation from source requires `make`
and `gcc`, but no external libraries.

    $ git clone https://github.com/gfetterman/grackle.git
    $ cd grackle
    $ make

The executable produced is named `grackle`; it is not installed globally.
