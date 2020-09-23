# cbi #

Your average bytecode interpreter. C++ IR/VM instead of C because at the moment I don't want to write up all the garbage collection or use non-growable arrays. Might change in the future.

NOTE: Lists sometimes do weird stuff. Dunno why. There are also other things that go weird. CBI is an exercise, and shouldn't be used for anything important.

## Working CBI ##

It's recommended to add cbi\bin to your PATH.

CBI is called through the command line:
```CMD
d:\...\cbi\examples> ccbi helloworld.cbi
Hello, World!
```
### Command Line Flags ###
-h -help
```CMD
d:\...\cbi\examples> ccbi -h
Documentation: https://github.com/LackingInGrav1tas/cbi
```

-b -build
```CMD
d:\...\cbi\examples> ccbi helloworld.cbi -b
Hello, World!
d:\...\cbi\examples> dir
...
build.bat
...
```

-d -debug
```CMD
d:\...\cbi\examples> ccbi helloworld.cbi -d
Reading file... 733 microseconds
Lexing file... 293 microseconds
Compiling opcode... 286 microseconds
Disassembling opcode...
== opcode ==
| byte 0000 lines -001 | OP_BEGIN_SCOPE
| byte 0001 lines 0000 | OP_CONSTANT  position: 0  value: std
| byte 0003 lines 0000 | OP_BEGIN_NAMESPACE
| byte 0004 lines 0000 | OP_CONSTANT  position: 1  value: OS_ANDR
| byte 0006 lines 0000 | OP_CONSTANT  position: 2  value: 0
| byte 0008 lines 0000 | OP_VARIABLE
| byte 0009 lines 0001 | OP_CONSTANT  position: 3  value: OS_FBSD
| byte 0011 lines 0001 | OP_CONSTANT  position: 4  value: 0
| byte 0013 lines 0001 | OP_VARIABLE
| byte 0014 lines 0002 | OP_CONSTANT  position: 5  value: OS_MAC
| byte 0016 lines 0002 | OP_CONSTANT  position: 6  value: 0
| byte 0018 lines 0002 | OP_VARIABLE
| byte 0019 lines 0003 | OP_CONSTANT  position: 7  value: OS_UNIX
| byte 0021 lines 0003 | OP_CONSTANT  position: 8  value: 0
| byte 0023 lines 0003 | OP_VARIABLE
| byte 0024 lines 0004 | OP_CONSTANT  position: 9  value: OS_WIN
| byte 0026 lines 0004 | OP_CONSTANT  position: 10  value: 1
| byte 0028 lines 0004 | OP_VARIABLE
| byte 0029 lines 0005 | OP_CONSTANT  position: 11  value: EXIT_FAILURE
| byte 0031 lines 0005 | OP_CONSTANT  position: 12  value: 1
| byte 0033 lines 0005 | OP_VARIABLE
| byte 0034 lines 0005 | OP_CONSTANT  position: 13  value: EXIT_SUCCESS
| byte 0036 lines 0005 | OP_CONSTANT  position: 14  value: 0
| byte 0038 lines 0005 | OP_VARIABLE
| byte 0039 lines 0006 | OP_CONSTANT  position: 15  value: assert
| byte 0041 lines 0006 | OP_DECL_FN with 0
| byte 0043 lines 0007 | OP_CONSTANT  position: 16  value: input
| byte 0045 lines 0007 | OP_DECL_FN with 1
| byte 0047 lines 0008 | OP_CONSTANT  position: 17  value: exp
| byte 0049 lines 0008 | OP_DECL_FN with 2
| byte 0051 lines 0009 | OP_CONSTANT  position: 18  value: println
| byte 0053 lines 0009 | OP_DECL_FN with 3
| byte 0055 lines 0009 | OP_END_NAMESPACE
| byte 0056 lines 0009 | OP_BEGIN_SCOPE
| byte 0057 lines 0010 | OP_CONSTANT  position: 19  value: Hello, World!
| byte 0059 lines 0010 | OP_PRINT_TOP
| byte 0060 lines 0011 | OP_END_SCOPE
| byte 0061 lines -001 | OP_END_SCOPE
== end ==

== constants ==
0: std
1: OS_ANDR
2: 0
3: OS_FBSD
4: 0
5: OS_MAC
6: 0
7: OS_UNIX
8: 0
9: OS_WIN
10: 1
11: EXIT_FAILURE
12: 1
13: EXIT_SUCCESS
14: 0
15: assert
16: input
17: exp
18: println
19: Hello, World!
== end ==

== runtime ==
Hello, World!
EXIT_OK
== end ==
Time spent in runtime: 571 microseconds.

== stack ==
== end ==
Total relevant time taken:
1883 microseconds
0.001883 seconds
```

## Grammar EBNF ##
```EBNF
<code> ::= <declaration>* ;

<declaration> ::= <statement> | ( <set-variable> | (("sleep"|"console"|"throw") <expression>) ";" ) | <fn-declaration> | <infix-declaration> | <prefix-declaration> ;

<statement> ::= (<expression> | <print-statement> | ("break" | "disassemble_constants" | "disassemble_stack" | "disassemble_scopes" | ("gets"|"getc" IDENTIFIER) ";")) | <if-statement> | <while-statement> | <code-block> ;
<set-variable> ::= "set" ["mut"] IDENTIFIER [":" <type-specifier>] [ "=" <expression> ] ;
<fn-declaration> ::= "fn" ["aware"|"blind"] IDENTIFIER "(" (IDENTIFIER ":" <type-specifier> [","])* ")" <code-block> ;
<infix-declaration> ::= "infix" IDENTIFIER "(" IDENTIFIER ":" <type-specifier> "," IDENTIFIER ":" <type-specifier> ")" "precedence" NUMBER <code-block> ; (* infix operators must have two params *)
<prefix-declaration> ::= "prefix" IDENTIFIER "(" IDENTIFIER ":" <type-specifier> ")" "precedence" NUMBER <code-block> ; (* prefix operators must have one param *)


<type-specifier> ::= "ANY"|"STR"|"NUM"|"BOOL"|"LIST"|"VOID" ;
<print-statement> ::= "print" <expression> ;
<if-statement> ::= "if" <group> <flexible-block> [ "else" <flexible-block> ] ;
<while-statement> ::= "while" <group> <flexible-block> ;

<flexible-block> ::= <code-block> | <declaration> ;
<expression> ::= <operation> | <literal> | <group> | <fn-call> | IDENTIFIER ;

<group> ::= "(" <expression> ")" ; (* 5*(2+8) *)
<fn-call> ::= "@" IDENTIFIER "(" <expression>* ")" ;    (* @foo() *)

<code-block> ::= "{" <code> "}" ;
<operation> ::= <infix> | <prefix> ;

<infix> ::= <expression> ("-" | "+" | "*" | "/" | ">" | "<" | "=" | ("!" | ">" | "<" | "||" | "=" | "+" |"-" | "*" |"/"  "=") | "push" | "and" | "or" | "at" | "index" | IDENTIFIER(* custom ops *) |  ) <expression> ;
<prefix> ::=  ("!" | "-" | "$" | "@"  | "rand" | "pop" | "ascii" |"sizeof" | "front" | "back" | IDENTIFIER(* custom ops *)) <expression> ;

<literal> ::= STRING | NUMBER | "true" | "false" | "null" | "list" ;
```

## Code Snippets ##

### Hello World: ###
```
print "Hello, World!";
```
output:
```
Hello, World!
```
### Variables: ###
```
set mut a; # similar to Rust, the variable is assumed to be immutable. mut specifies it's state.
a = 729;
set b: NUM = 2600; # b is required to be a number value
print $a * $b; # $ retrieves the variable's value
```
output:
```
1895400
```
### Control Flow ###
```
set mut a; # scope is accessable through the if statement
set condition = 3;
if (condition == 3) // like C family languages, brackets are not needed for one line blocks.
    a = "This will print.";
else if (condition == 2)
    a = "This won't."
else 
    a = "Neither will this.";

print $a;
```
output:
```
This will print.
```
while loop:
```
set mut i = 0;
while ($i < 5) {
    i += 1;
    print $i || "\n";
}
```
or
```
fn body() {
    i += 1;
    print $i || "\n";
}
set mut i = 0;
while ($i < 5) @body();
```
output:
```
1
2
3
4
5

```
### Functions ###
```
fn getnum(x: NUM) { # type specifiers are necessary. ANY can be used as a template/generic
    return $x * 3.78;
}

print @getnum(45);
```
output:
```
170.1
```
Scope can be modified by using the ```aware``` or ```blind``` keywords after ```fn```. When a function is called without these keywords, it is given the first layer of scope to access/modify. When a function is called with ```aware```, the function is given full access to the scope at the point of the function call. ```blind``` functions aren't given any scope at all.

The way that parameters are handled can lead to strange errors, ex:
```
fn getnum(x: NUM) {
    return $x * 3.78;
}

print 45 + @getnum();
```
output:
```
Run-time Error in line 4: Stack underflow.
```
The stack underflow happens when handling OP_ADD. This occurs because of how function calls are compiled. The compiler doesn't check for arity because functions can be called before they are defined. When the 45 is found by the compiler, it is pushed to the opcode as a OP_CONSTANT number value. It is then taken as the parameter for OP_CALL on getnum (When a call is reached it assumes that it's params are on the stack). After this, OP_ADD finds that there is only 1 value on the stack, causing the error. This could be fixed with a seperate param stack, but isn't really necessary.
### User Input ###
gets:
```
set mut input; #needs to be mutable
gets input;
print $input;
```
Sets the variable input to the user's input.
```gets``` is handled by (and works similar to)
```C++
std::string input;
std::getline(std::cin, input);
```
```getc``` works the same as ```gets``` in cbi, but is handled differently in runtime. ```getc``` uses
```C++
(char)getch()
```
The important difference is that ```getc``` has no buffer, whereas ```gets``` needs to find the enter key to return.
### Conversion ###
The built-in type conversion uses ```as``` and type specifiers (```NUM```, ```STR```, ```BOOL```, ```LIST```, ```VOID```, ```ANY```).

as
```
print 50 * "46.4" as NUM;
```
output:
```
2320
```
```as```'s precedence is greater than ```+```/```-``` and ```*```/```/```.
### Custom Operators ###

NOTE: Because of how custom operators work, they cant be called from namespaces by ```namespace::op``` because of bad foresight. Might be fixed soon. Just do ```use namespace``` and it works fine.

Custom operator declarations work very similar to function declarations, as they are mostly just retooled functions. The main difference between operators and functions, besides the obvious, is that the operator's precedence is needed during compile time. This means that their implementations are somewhat different.

custom infix operator (there must be two params because of how infix operators works):
```
infix exp(lhs: NUM, rhs: NUM) precedence 5 {
    set mut i = 1;
    set base = $lhs;
    while ($i < $rhs) {
        lhs *= $base;
        i += 1;
    }
    return $lhs;
}

print 5 exp 3;
```
output:
```
125
```
custom prefix operator (there must be one param):
```
prefix println(rhs: STR) precedence 1 {
    print $rhs || "\n";
}

println "abc";
```
output
```
abc

```
### Lists ###

NOTE: Lists show some weird / program breaking behavior

Lists are first class objects.
List initialization:
```
set mut list_obj: LIST = list; # 'list' works like 'null', it returns a blank object.
set list_obj = list(1, 2, 3); # 'list' can also do this
```
pushing:
```
list_obj push "This, for example.";
```
popping:
```
pop list_obj;
```
back:
```
print back $list_obj;
```
front:
```
print front $list_obj;
```
both output
```
This, for example.
```

Example program:
```
set mut L = list;
L push "front";
L push 35;
L push null;
set L[2] = "back"; #type specifiers do work
println $L[2];
println back $L;
println front $L;
println $L as STR;

print "abcd" as LIST;
```
```
back
back
front
list(front, 35, back)
list(abcd)
```

### Utility ###
sizeof (not like C's sizeof()):
```
print "sizeof (on string): " || sizeof "this" || "\n" ;
set mut list_example = list;
list_example push 0;
print "sizeof (on list): " || sizeof $list_example;
```
```
sizeof (on string): 4
sizeof (on list): 1
```

ascii:
```
print "NUM to STR: " || ascii 97 || "\n";
print "STR to NUM: " || ascii "a";
```
```
NUM to STR: a
STR to NUM: 97
```
console:

Using C++'s ```system```, it sends a command to the console.
```
console "mkdir new_folder"; # creates folder
console 'echo "blah, blah, blah" > file.txt'; # writes to file
```
rand:

When called during runtime, it seeds the PRNG (using ```time(NULL)```) and returns a number in between 0 and it's param.
```
print rand 50;
```
```
<a number between 0 and 49>
```
sleep:

Waits for it's param.
```
println "Wait for two seconds..."; # println is in the STL
sleep 2000;
print "Done.";
```
```
Wait for two seconds...
Done.
```
throw:

uses the ```ERROR()``` definition from runtime.cpp on ```getPrintable(top)```, then exits with errorcode 1;
```
throw "Get me outta here!";
```
```
Run-time Error in line 1: Get me outta here!
```

floor:
```
print floor 45.9;
```
```
45
```
### Namespaces ###

creating a namespace
```
namespace ns {
    set variable = "var"; # can also be created by set ns::variable
    fn foo() { # can also work with fn ns::foo()
        println "in foo";
    }
}
```

interacting with a namespace
```
println $ns::variable;
@ns::foo();

use ns;
println $variable;
@foo();
```
```
var
in foo
var
in foo
```

## The Standard Library ##
The way I implemented the STL is inefficient because I don't want to write the opcode by hand. 
ALL THINGS IN THE STANDARD LIBRARY EXIST BEHIND THE ```std``` NAMESPACE

### Constants ###
```EXIT_SUCCESS```: ```set EXIT_SUCCESS = 0;```
```EXIT_FAILURE```: ```set EXIT_FAILURE = 1;```

```OS_WIN```:
```C++
#ifdef _WIN32
    lines.insert(lines.begin(), "set OS_WIN = true;");
#else
    lines.insert(lines.begin(), "set OS_WIN = false;");
#endif
```
```OS_UNIX```:
```C++
#if defined(unix) || defined(__unix__) || defined(__unix)
    lines.insert(lines.begin(), "set OS_UNIX = true;");
#else
    lines.insert(lines.begin(), "set OS_UNIX = false;");
#endif
```
```OS_MAC```:
```C++
#ifdef __APPLE__
    lines.insert(lines.begin(), "set OS_MAC = true;");
#else
    lines.insert(lines.begin(), "set OS_MAC = false;");
#endif
```
```OS_FBSD```:
```C++
#ifdef __Free_BSD__
    lines.insert(lines.begin(), "set OS_FBSD = true;");
#else
    lines.insert(lines.begin(), "set OS_FBSD = false;");
#endif
```
```OS_ANDR```:
```C++
#ifdef __ANDROID__
    lines.insert(lines.begin(), "set OS_ANDR = true;");
#else
    lines.insert(lines.begin(), "set OS_ANDR = false;");
#endif
```
### Functions ###
```assert```:
```
fn assert(expr: BOOL) {
    if (!$expr) throw "assertion failed.";
}
```
```input```:
```
fn input(text: ANY) {
    print $text;
    set mut s;
    gets s;
    return $s;
}
```
### Operators ###
```println```:
```
prefix println(txt: ANY) precedence 1 {
    print $txt || "\n";
}
```
```exp```:
```
infix exp(lhs: NUM, rhs: NUM) precedence 5 {
    set mut i = 1;
    set base = $lhs;
    while ($i < $rhs) {
        lhs *= $base;
        i += 1;
    }
    return $lhs;
}
```
