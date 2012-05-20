                                 Sally
                       A Strongly-Typed Language
                       -------------------------
   v2000.02.26 (c)2000 Cat's-Eye Technologies.  All rights reserved.

What is Sally?
--------------

Sally is basically FORTH, except:

- All functions are declared with fixed range and domain types
- Strong type checking is applied to arguments and results
- User-defined types can be introduced into the checking scheme
- Forward, instead of reverse, Polish notation is used

Sally is also an exercise in the reduction of excise (redundant,
extraneous, or sugary syntax).

For example, there is, unlike FORTH, no special symbol to indicate
the end of a function definition.  (A new definition starts whenever
the previous one ends.)

The ANSI C source code for the compiler is very small, less than
16K in total.  The compiler translates Sally programs to ANSI C.

Just to be silly I've scattered exercises for the reader around the
documentation.  I don't actually expect anyone to try any of them,
but they kind of indicate what I'd like to see in a "Sally++" if
there ever was such a beast.

Syntax
------

A Sally program consists of a series of function declarations.
Each declaration consists of the name and type of a new function
followed by a series of function applications and primitives.

The domain types are listed before the function name, the range
types, after.  For example, a function called foo which maps an
integer and a character to an integer, would be notated as

  int char foo int ...

The '...' represents the function applications which compose the
body of the function.

The syntax of each application is dictated by the types given in
the function's definition.  For example, a correct syntax for
applying the 'foo' function above might be

  foo 42 'X

(Note the lack of a need for parentheses.)  This application of
foo would be appropriate anywhere a previous application requires
an integer argument, or where a definition requires an integer
result.

Functions can accept multiple arguments and return multiple results.
A function which returns multiple results can pass those results as
a 'chunk' to the previous function application.  For example, the
following would return the double of the given argument '$1':

  add dup $1

And this would probably appear in a function definition like so:

  int double int add dup $1

The type 'void' indicates lack of any arguments or results.  When
it indicates the lack of arguments, it must be specified explicitly
(to indicate to the parser that the previous definition has ended.)
When used to indicate the lack of results, it can be implicitly
inferred by the fact that there are no types following the function
name.

EXERCISE:  See if you can find a way to change the parser so that
'void' is always implicit, even when it's used to indicate the
lack of arguments to a function.  Be prepared to deal with a token
which is an undeclared symbol in a mature way.

Functions which Accept and Return Functions
-------------------------------------------

A function can be passed to another function.  In order for a
function to be defined which accepts a function passed to it, a
prototype of the type of function to be passed must be defined.
This prototype is then preceded in the type declaration by the
'like' operator.

For example, say we want a function which accepts an integer
and a function which maps an integer to another integer, and
returns an integer.  To do this, first we establish the prototype
function which is included in the arguments to the other function:

  int map int proto

(This kind of definition, a "proto", also functions like a "forward"
declaration in Pascal or an "extern" declaration in C.)

We then use the 'like' operator in specifying the definition of the
other function:

  int like map other int ...

The function 'other' now takes an integer and a function 'like map'
(that is, a function which has the same domain and range types as
the function called 'map', even if the body of 'map' is not actually
defined) and returns an integer.

Even so, how does an application pass a function to another function?
You can't just name the function, because that indicates that you want
to apply it and use the return value.  You must instead "reference" or
"escape" the function by preceding it with "the".  So to apply the
'other' function above to a function such as 'negate', one would write

  other 5 the negate

Assuming 'negate' is declared with the same range and domain types as
'map', this should pass the function 'negate' without trying to apply
it (presumably leaving that up to the 'other' function).

Speaking of which, there is one last issue to cover.  Once a function
like 'other' has a reference to a function like 'map' passed to it,
how does 'other' use the passed function?

The answer is the "do" keyword.  "do" is followed by the number of
an argument.  It is not unlike $1, but it does not simply evaluate to
the argument, it actually calls it.  When this is the case, the
argument better be a function (all that strong typing stuff applies.)

EXERCISE:  See if you can extend the "the" operator to provide
lambda functions.  Use the following as a syntax guideline for how
one would specify a lambda function:

  other 5 the int -> int + $1 7

Remember, there's no special "End Function Definition" symbol!

EXERCISE:  Extend "do" to handle "the" functions directly.  Example:

  do the monkey

EXERCISE:  Finally, extend "like" to likewise handle "the" functions
directly.  Example:

  int like the int -> int proto other int ...

Type Checking and Casting
-------------------------

A named type can defined in Sally by defining it like you would define
a function, but with no range type, and with no definition, instead the
token 'type', as in:

  int semaphore type

In Sally, two types are equivalent if:
- Both are named and their names are the same; or
- Neither is named and they are structurally equivalent.

(By named, I of course mean, named by the user, not intrinsic to the
language.  Even though 'int' is technically a name, it's unnamed for
our purposes here.)

Values can be cast from one type to another.  This is invaluable for
any structured data access in Sally.

No conversions are done during type casting.  Types can only be
cast to other types of the same size (number of elements - you can
cast 'char' to 'int' but not to 'int int' or 'void'.)

Typecasts are performed with the "as" operator, as in

  as char 7

to represent a bell.

EXERCISE:  Implement type variables, so that one could define
functions like

  `1 `2 swap `2 `1 $2 $1

Which would be applicable no matter what types were passed to and
received from 'swap', as long as the types were consistent (all
`1's are the same type and all `2's are the same type for any
given application of 'swap'.)

There are several ways one may want to attempt this.  One is by
using the process of 'unification' to make sure all type variables
are used consistently.  Adding this to Sally may be excessively
tricky because of the way it does type checking.  With a stack of
types, there may be a more efficient algorithm for replacing type
variables with instances, and subsequently checking them for
consistency.

Libraries, Side Effects, and main
---------------------------------

Libraries can be included before the first function definition in a
program.  There is no special "include" token, the library is simply
named, and if a file named that (plus the extension .sal) is located,
it is (virtually) inserted into that point in the program.

For example, many of the sample programs begin with the token
'stdlib'.  This loads the file stdlib.sal into that point in the
program, introducing a handful of function prototypes.

Libraries are also how Sally deals with side effects.  The philosophy
is that if the programmer wants to disturb the purity of the functional
paradigm by introducing side effects, they may do so, but they will be
made clearly aware of the fact.

Having said that, Sally can only perform side effect communications
by including the library called 'sidefxio' - thus reminding the
programmer that there are side effect communications at work in the
following program.

Without including this library Sally is limited to batch
communications.

In both schemes, the function called 'main' is the function which is
applied when the entire program is run.  'main' may have any number
of arguments and any number of return values, although they may only
be of "int" type.  For each argument, a value is required as a
command-line argument when the program is run; for each result, the
resultant value is displayed at the end of the program.

This little communications scheme is minimal, and limited, but it
does not introduce any side effects in any way, and is capable of
communicating any Turing-solvable problem, so it has some things
going for it.  To get around the limitations, however, you have to
resort to side effect I/O, where you can use the 'print' and
'input' functions to report and obtain information from the user
while the program is running.

EXERCISE:  Loosen the constraints on the type of the 'main'
function - allow arguments and return values of type 'char'.

EBNF Grammar
------------

  Program     ::= {Definition}.
  Definition  ::= {Library} {Type} Name {Type}
                  ("type" | "proto" | {Application}).
  Type        ::= "int" | "char" | "like" Name | Name.
  Application ::= Primitive
                | "$" Number
	        | (Name | "do" Number) {Instr}
                | "as" {Type} Instr
	        | "if" Instr Instr Instr
                | "the" Name.
  Primitive   ::= <<an integer notated in decimal like 123>>
                | <<a character preceded by a single quote like 'A>>.
