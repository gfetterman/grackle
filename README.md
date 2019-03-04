# Grackle

`Grackle` is a command-line interpreter for a (very) limited subset of the
[Racket](https://racket-lang.org) dialect of Lisp.

This is a hobby project, with features being added as I figure them out, and 
have time.

## Supported language features

* arbitrarily-nested s-expression arithmetic
* variable definition
* list manipulation
* predicate and conditional expressions

## Next language feature to be added

* user-defined functions

## Installation

Binaries are not currently available. Compilation from source requires `make`
and `gcc`.

    $ git clone https://github.com/gfetterman/lisp_interpreter.git
    $ cd lisp_interpreter
    $ make

The executable produced is named `grackle`; it is not installed globally.
