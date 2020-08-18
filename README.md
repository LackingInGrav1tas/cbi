# cbi #

Your average bytecode interpreter. C++ IR/VM instead of C because at the moment I don't want to write up all the garbage collection or use non-growable arrays. Might change in the future.
Very much in development.


## Grammar EBNF ##
```EBNF
<code> ::= <declaration>* ;

<declaration> ::= <statement> | <set-variable> ;

<statement> ::= (<expression> | <print-statement> ";") | <if-statement> | <code-block> ;
<set-variable> ::= "set" ["mut"] IDENTIFIER [ "=" <expression> ] ";" ;

<print-statement> ::= "print" <expression> ;
<if-statement> ::= "if" <group> <flexible-block> [ "else" <flexible-block> ] ;

<group> ::= "(" <expression> ")" ;
<flexible-block> ::= <code-block> | <declaration> ;
<expression> ::= <operation> | <literal> | <group> ;

<code-block> ::= "{" <code> "}" ;
<operation> ::= <infix> | <prefix> ;

<infix> ::= <expression> "-" | "+" | "*" | "/" | "||" | "==" | "!=" | ">" | ">=" | "<" | "<=" | "=" <expression> ;
<prefix> ::= <get-var> | ("!" | "-" <expression>) ;
<get-var> ::= "$" IDENTIFIER ;

<literal> ::= STRING | NUMBER | "true" | "false" | "null" ;
```

## Code Snippets ##

### hello world: ###
```
print "Hello, World!";
```
output:
```
Hello, World!
```
### variables: ###
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
### control flow ###
```
set mut a; # scope is accessable through the if statement
if (true) {
    a = "This will print.";
} else {
    a = "This won't."
}
print a;
```
output:
```
This will print. So will this.
```
