# cbi #

Your average bytecode interpreter. C++ IR/VM instead of C because at the moment I don't want to write up all the garbage collection or use non-growable arrays. Might change in the future.
Very much in development.


## Grammar EBNF ##
```EBNF
<code> ::= <declaration>* ;

<declaration> ::= <statement> | ( <set-variable> ";" ) | <fn-declaration> ;

<statement> ::= (<expression> | <print-statement> | ("break" | "disassemble_constants" | "disassemble_stack" | "disassemble_scopes" | ("gets"|"getc" IDENTIFIER) ";")) | <if-statement> | <while-statement> | <code-block> ;
<set-variable> ::= "set" ["mut"] IDENTIFIER [ "=" <expression> ] ;
<fn-declaration> ::= "fn" IDENTIFIER "(" (IDENTIFIER [","])* ")" <code-block> ;

<print-statement> ::= "print" <expression> ;
<if-statement> ::= "if" <group> <flexible-block> [ "else" <flexible-block> ] ;
<while-statement> ::= "while" <group> <flexible-block> ;

<flexible-block> ::= <code-block> | <declaration> ;
<expression> ::= <operation> | <literal> | <group> | <fn-call> | IDENTIFIER ;

<group> ::= "(" <expression> ")" ; (* 5*(2+8) *)
<fn-call> ::= "@" IDENTIFIER "(" <expression>* ")" ;    (* @foo() *)

<code-block> ::= "{" <code> "}" ;
<operation> ::= <infix> | <prefix> ;

<infix> ::= <expression> ("-" | "+" | "*" | "/" | "||" | "==" | "!=" | ">" | ">=" | "<" | "<=" | "=" | ("+"|"-"|"*"|"/"|"||") "=" ) <expression> ;
<prefix> ::= <get-var> | ("!" | "-" <expression>) ;

<get-var> ::= "$" IDENTIFIER ; (* $var_name *)

<literal> ::= STRING | NUMBER | "true" | "false" | "null" ;
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
set b = 2600; # immutable
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
fn getnum(x) {
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
fn getnum(x) {
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