# Semantic Rules


# 1 . Type errors
## i. variable type check

* Assignment of values of different types

#### correct syntax

```
func greet() => void {
    local age : integer;
    local marks : float;

    age := 10;
    marks := 10.5;
}

```
#### rise error when 
```
func greet() => void {
    local age : integer;
    local marks : float;

    age := 10.5;
    marks := 10;
}

```
#### output 
```
Type error: Cannot assign float to variable 'age' of type integer at line 5, col 5
Type error: Cannot assign integer to variable 'marks' of type float at line 6, col 5
```
## ii. function return type check
* Incorrect use of return statements
* 
#### correct syntax
```
func greet() => integer {
   return (1);
}
```
#### rise error 
```
func greet() => integer {
   return (1.5);
}
```
#### output 
```
Semantic error: Return type mismatch in function 'greet'. Expected 'integer' but got 'float' at line 2, col 4
```
# 2. Scope Errors

#### i. use of undeclared identifier

#### correct code

```
func main() => void {
   local a : integer;
   a := 5;
}
```
#### rise error when

```
func main() => void {
   a := 5;
}
```
#### output
```
Semantic error: Undeclared variable 'a' at line 3, col 4
```
#### iii. duplicate declarations of identifiers

#### iv. Declaration of Class before Implementation

#### correct code
```
class Car {
// function declaration

   public func printDetails(NoOfSeats : integer) => integer;
};

implement Car {
// function definition/implementation

 func printDetails(NoOfSeats : integer) => integer {
  return (1);
 }
 
}
```

#### rise error when

```

implement Car {
// function definition/implementation

 func printDetails(NoOfSeats : integer) => integer {
  return (1);
 }
 
}
```

```
Semantic error: Implementation for unknown class 'Car'
```

## 3. Update function satus in symbol table

* Before the function definition ST declared = "yes"
* After the function definition ST declared = "no"


```
class Car {
// function declaration

   public func printDetails(NoOfSeats : integer) => integer;
};

implement Car {
// function definition/implementation

 func printDetails(NoOfSeats : integer) => integer {
  return (1);
 }
 
}
```





