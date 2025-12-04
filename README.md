# grblang

`gr`[ian's]`b`[ytecode]`lang` is a scripting language that compiles to a custom bytecode.

## Data Types

The currently supported basic data types are `int`, `bool`, `string`.

### Arrays

Arrays types are declared using this syntax `int[][][]`. They do not accept or require a length.

## Syntax

An important distinction is that all statements must end in a `;`, including if and while statements.

### Arrays

Arrays can be declared as follows:
`[1, 2, 3, 4]`

Arrays must contains elements of the same type.

To append to arrays, use the following syntax:
```
arr + 3;
```

This can be then assigned back to the array's variable or used as a expression.

### Variables
Variable declarations must be prefixed with the keyword `var` and then the type. 
```
var int x = 3;
```

Variables can be reassigned with the following syntax:
```
x = 4;
```

Variables can be referenced simply with the identifier: `x`

### If statements

If statements have the following syntax:

```
if (1 == 2) { 
    // code
};```

Optionally, there can also be an else block
```
if (1 == 2) {
    // code
} else {
    // other code
};
```

### While Statements

While statements have the following syntax

```
while (1 == 2) {
    // code to repeat
};```

### Functions

`/!\ Warning: Functions are WIP and unfinished at the time of writing`

Functions use the following syntax:
```
fn int x(int y) {
    return y+4;
};

x(4);
```
Function declarations must be prefixed with `fn` and then the return type, following that the function name, then the list of parameters with their types.
All functions must include a `return` statement.
