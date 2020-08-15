# cbi #

Your average bytecode interpreter. C++ IR/VM instead of C because at the moment I don't want to write up all the garbage collection or use non-growable arrays. Might change in the future.
Very much in development.


## Grammar EBNF ##
```EBNF
<code> ::= <declaration>* ;

<declaration> ::= <statement> | <set-variable> ;

<statement> ::= (<expression> | <print-statement> ";") | <if-statement> | <code-block> ;
<set-variable> ::= "set" ["mut"] IDENTIFIER [ "=" <expression> ] ";" ;

<print-statement> ::= "print" [<expression>] ;
<if-statement> ::= "if" <group> <flexible-block> [ "else" <flexible-block> ] ;

<group> ::= "(" <expression> ")" ;
<flexible-block> ::= <code-block> | <declaration> ;
<expression> ::= <operation> | <literal> | <group> ;

<code-block> ::= "{" <code> "}" ;
<operation> ::= <infix> | <prefix> ;

<infix> ::= <expression> "-" | "+" | "*" | "/" | "||" | "==" | "!=" | ">" | ">=" | "<" | "<=" | "=" <expression> ;
<prefix> ::= <get-var> | ("!" | "-" <expression>) ;
<get-var> ::= "$" IDENTIFIER ;

<literal> = STRING | NUMBER | "true" | "false" | "null" ;
```

## Code Snippets ##

### hello world: ###
```Batch
print "hello world!";
```
output:
```
hello world!
```
### variables: ###

![Alt text](examples/variables_example.png?raw=true "Title")

output:
```
1462374
```
### control flow ###
if:
```
if (true) print "This will print.";
if (true) {
    set a = " So will this.";
    print $a;
}
if (false) print "This won't print.";
if (false) {
    set a = " Neither this.";
    print $a;
}
```
output:
```
This will print. So will this.
```