# cbi #

Your average bytecode interpreter. C++ IR/VM instead of C because at the moment I don't want to write up all the garbage collection or use non-growable arrays. Might change in the future.
Very much in development.


## Grammar EBNF ##
```EBNF
<declaration> = <statement> | <set-variable> ;

<statement> = <expression> | <print-statement> ";" ;
<set-variable> = "set" ["mut"] IDENTIFIER [ "=" <expression> ] ";" ;

<print-statement> = "print" [<expression>] ;

<expression> = <operation> | <literal> | <group> ;

<operation> = <infix> | <prefix> ;

<infix> = <expression> "-" | "+" | "*" | "/" | "||" | "==" | "!=" | ">" | ">=" | "<" | "<=" | "=" <expression> ;
<prefix> = <get-var> | ("!" | "-" <expression>) ;
<get-var> = "$" IDENTIFIER>;

<literal> = STRING | NUMBER | "true" | "false" | "null" ;
<group> = "(" <expression> ")" ;
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
### variable stuff: ###

<img width="401" alt="variables_example" src="https://user-images.githubusercontent.com/42680395/90297234-d418b280-de5b-11ea-9d29-235f253ec382.png">

output:
```
1462374
```
