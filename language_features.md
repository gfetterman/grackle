# grackle language features

## Types

### Pair & list

Pairs may be constructed using the `cons` special form. Lists are constructed as
usual for Lisp dialects using nested `cons` cells. A list is terminated by the
empty list, which may be represented using the symbol `null`.

The `pair?` and `list?` predicates may be used to test values.

The first and last portion of a `cons` cell is accessed using the `car` and
`cdr` functions, respectively.

A list may be created more conveniently using the `list` function.

### Fixed-precision integer

Integer values are represented in fixed-precision - Racket's arbitrary precision
integers are not yet supported.

Integer precision is fixed at the size of a `long int`, which on most platforms
is between `-(2**31)` and `2**31 - 1`. Arithmetic or literal input which
exceeds these values results in an error.

Operations on integers include arithmetic (`+`, `-`, `*`, `/`) and comparison
(`=`, `<`, `>`, `<=`, `>=`).

Floating-point numbers are not currently supported.

Division is integer division truncated towards zero.

Division by zero results in an error.

A number value may be tested using the `number?` predicate.

### Boolean

Literal boolean values may be specified in the REPL using `#t` for true and `#f`
for false. These values should not be used as symbol names.

Boolean values may be tested using the `boolean?` predicate.

Only a boolean false value counts as false in grackle, as in Racket. All other
values, including `void`, 0, and the empty list (i.e., `null` or `'()`), count
as true.

### String

String literals can be typed in the REPL between double quotation marks (`"`).
The only symbol which requires escaping within a string is the double quotation
mark; it may be escaped with a backslash ('\').

String encoding is currently only ASCII.

There are currently only a handful of string manipulation functions built in:

* `string?`
* `string-length`
* `string=?`
* `string-append`

### User-defined procedure

Functions may be defined using either `define` or `lambda` and passed around as
first-class objects. Function names occupy the same namespace as symbols, and
have the same restrictions on characters.

Function values may be tested using the `procedure?` predicate.

### Symbol

Bare symbols, printed `'<<name>>`, can be passed around using the `quote`
special form. When a symbol is evaluated, if it is undefined in the current
scope, an error is raised. grackle 

Symbol names can include any character except `(`, `)`, or `"`. This behavior
differs from Racket: grackle allows more characters usually preserved in Racket
(such as `#`), but for those characters that it prohibits, there is no way to
escape them as there is in Racket.

There is no restriction on symbol length. There is currently a limit to the
number of unique symbols which may appear in a given grackle session, but it is
the maximum positive integer which can fit into an `unsigned int`, which is, for
most consumer systems, `2**32 - 1`. Creating more symbols than this will result
in undefined behavior.

The `symbol?` predicate determines whether a given object is a symbol.

### Built-in procedure

Built-in functions and special forms, like `+` or `define`, are represented
differently from user-defined functions, but they behave more or less
identically.

Built-in functions and special forms count as procedures for the `procedure?`
predicate.

### Void

Some built-in functions or special forms can return a `<void>` value if there is
no meaningful return value. These include:

* `(define ...)`
* `(set! ...)`
* `(cond ...)` if there is no `else` clause, and no clause predicates evaluate
  to non-false

A value may be tested to be void using the `void?` predicate.

## Other features

### Boolean expressions

The `and` special form evalutes each argument in order until it reaches one that
evaluates to `#f` or it runs out of arguments. If it reaches `#f`, no further
arguments are evaluated, and it returns `#f`. If it runs out of arguments, the
value of the last argument's evaluation is returned.

The `or` special form works in the same way, but it short-circuits out when it
encounters a non-`#f` value or runs out of arguments.

The `not` function returns `#t` when its argument evaluates to `#f`, and `#f`
otherwise.

### Mutation

The built-in special form `set!` will change the value of an already-defined
variable. Using it on an undefined variable results in an error.

### REPL exit

The built-in command `exit` will exit the REPL back to the command-line.

### Conditional expression

The built-in special form `cond` works the same way as it does in Racket: it
takes zero or more lists of items and treats them in order. If the first item in
a list evaluates to not-`#f`, the subsequent items in the list are evaluated,
and the last item evaluated is returned. If the first item in the list evaluates
to `#f`, execution skips to the next list. If no list's first element evaluates
to not-`#f`, `<void>` is returned.

However, if the first element of the last list is the symbol `else`, that counts
as a non-`#f` value, and execution passes to the subsequent elements in that
list. Any list other than the last being headed by `else` produces an error.
