# Custom Regex Engine

This project is a custom regular expression (regex) engine written in C, implementing several key algorithms to efficiently process and match regular expressions. The engine supports the most fundamental set of regex operations and syntax.

## Features

1. **Infix to Postfix Conversion**: 
   - Utilizes the Shunting Yard Algorithm by Edsger Dijkstra to convert infix regular expressions to postfix notation. This step ensures that the expressions are in a form that can be easily processed by the subsequent stages of the engine.

2. **NFA Construction**:
   - Constructs a Non-deterministic Finite Automaton (NFA) from the postfix regular expression using Ken Thompson's Algorithm. This approach, described in his seminal 1968 paper, allows for the efficient creation of NFAs that can represent the various regex operations.

3. **Multiple-State Simulation**:
   - Employs the multiple-state simulation technique also outlined in Thompson's 1968 paper. This method enables the engine to achieve linear runtime performance by simultaneously tracking multiple states during the matching process.

## Supported Syntax

- `.` : Concatenation of expressions
- `*` : Matches zero or more occurrences of the preceding element
- `?` : Matches zero or one occurrence of the preceding element
- `+` : Matches one or more occurrences of the preceding element
- `|` : Matches either of the two expressions (alternation)

## Installation

Clone the repository and compile the source code using a C compiler:

```sh
git clone https://github.com/yourusername/custom-regex-engine.git
cd RegexEngine
make
```

## Usage

To use the regex engine, run the compiled binary with a regular expression and a string to match:

1. Import header file
2. Compile Regex expression
3. Match will return 0 or 1 when given a String

### Example

Given the regular expression `a.b|c*`, the engine will:

1. Convert the infix expression to postfix notation.
2. Construct an NFA representing the expression.
3. Simulate the NFA to determine if the input string matches the regular expression.

```c
#include "RegexEngine/regex_engine.h"

int main(int argc, char** argv){
   compile("a.b|c*");
   printf("output: %d", match("ab");
}
```

This command will output whether the string matches the regular expression.

---

For more detailed information, refer to the source code comments.

## Credit

Implementation based on and inspired by Russ Cox in his article [ Regular Expression Matching Can Be Simple And Fast (but is slow in Java, Perl, PHP, Python, Ruby, ...)](https://swtch.com/~rsc/regexp/regexp1.html). Many Thanks.
