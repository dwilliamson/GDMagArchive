
C++ Scripting System Using Expression Predicates

EmilDotchevski@hotmail.com



COMPILER COMPATIBILITY

To best of my knowledge, this source code is ANSI C++ compliant.
It has been explicitly tested with the following compilers:

Microsoft C++ Compiler, version 12 and 13 (MSVC 6 and 7)
Intel C++ Compiler, version 7.0
g++, version 3.2
Comeau Computing C++ Compiler, version 4.3.0.1



ADDITIONS TO THE ARTICLE MATERIAL

This example source code is more advanced than the code snippets given
in the article. Most notably, select_root and select_child templates
are extended to produce valid results when used with types containing
the const modifier.



SUGGESTED IMPROVEMENTS

The predicate system is independent of the container class used to
store pointers. As mentioned in the article, using std::set is not
ideal. I would recommend using a custom-designed container based on
std::vector, with efficient usage of reserve().

It is a good idea to not store pointers in the sets used to process
objects. For example, it is possible to use the so-called weak
pointers. The C++ Boost library provides an excellent implementation
of weak pointers (http://www.boost.org).



ACKNOWLEDGEMENTS AND LINKS

Todd Veldhuizen
"Expression Templates"
www.osl.iu.edu/~tveldhui/papers/Expression-Templates/exprtmpl.html

Bill Gibbons
"A Portable typeof Operator"
http://www.accu-usa.org/2000-05-Main.html

M. H. Austern
"Why You Shouldn't Use set - and What You Should Use Instead"
C++ Report 12:4, April 2000

Krzysztof Czarnecki and Ulrich W. Eisenecker
"Generative Programming - Methods, Tools, and Applications"
http://www.generative-programming.org

C++ Boost
http://www.boost.org

The Lambda Library
http://lambda.cs.utu.fi

Comeau C/C++ Compiler
http://www.comeaucomputing.com
