# Build
### Windows:

    mingw32-make

### Linux:

**Currently unavailable.**

### MacOS:

**Currently unavailable.**

# Usage

### Operators in descending order of their precedence:
1. **'^'**
2. **'\*', '/', '%'**
3. **'+', '-'**

* **-Use "ans" to reference the result of the previous expression.**

### You can use several math functions by escaping like this:
    \sqrt(num)
    
    \sin(num)

    \cos(num)

    \tan(num)

    \min(num1, num2)

    \max(num1, num2)

### You can declare variables like this:
    let var_name = number

### And use them inside the expressions like this:
    $var_name

-The variables are currently immutable- 

**Provide the 'test' flag if you want to run tests**

    ./TermCalc test
