# cbi #

Your average bytecode interpreter. C++ IR/VM instead of C because at the moment I don't want to write up all the garbage collection or use non-growable arrays. Might change in the future.
Very much in development.


## Grammar ##
```
DECLARATION = ( STATEMENT | SET_VARIABLE ) ";"

STATEMENT = EXPRESSION | PRINT_STATEMENT
SET_VARIABLE = "SET" "MUT"? IDENTIFIER ( "=" EXPRESSION )?

PRINT_STATEMENT = "PRINT" EXPRESSION?

EXPRESSION = OPERATION | LITERAL | GROUP

OPERATION = INFIX | PREFIX

INFIX = EXPRESSION "-" | "+" | "*" | "/" | "||" | "==" | "!=" | ">" | ">=" | "<" | "<=" EXPRESSION
PREFIX = "$" | "!" | "-" EXPRESSION

LITERAL = STRING | NUMBER | TRUE | FALSE | NULL
GROUP = "(" EXPRESSION ")"
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
```Batch Rust
set mut a; #like Rust, cbi assumes that the variable is immutable
a = 729;
set b = 2006; #immutable
print $a * $b;
```
output:
```
1462374
```
