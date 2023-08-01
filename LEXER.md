# Lexer

The lexer works in 2 layers. The first outlined with a state machine here builds an accumulator and passes that to the second layer. This second layer matches the accumulator to a variety of regexes that form the actual tokens.

```mermaid
---
title: Layer 1 of Lexer
---
stateDiagram-v2

    Space
    Word
    Number
    Comment
    Operator
    Other

    Space --> Space   : [\t|\ n| ]
    Space --> Number  : [0-9]
    Space --> Word    : [a-z|_|A-Z]
    Space --> Comment : [#]
    Space --> Operator: [+|*|/|-|=|(|)|[|]|{|}|<|>]
    Space --> Other   : [Anything else]

    Word  --> Space   : [\t|\ n| ]
    Word  --> Word    : [a-z|_|A-Z|0-9]
    Word  --> Comment : [#]
    Word  --> Operator: [+|*|/|-|=|(|)|[|]|{|}|<|>]
    Word  --> Other   : [Anything else]

    Number --> Space  : [\t|\ n| ]
    Number --> Comment: [#]
    Number --> Word   : [a-z|_|A-Z]
    Number --> Number : [0-9|.|e|E]
    Number --> Operator: [+|*|/|-|=|(|)|[|]|{|}|<|>]
    Number --> Other  : [Anything else]

    Comment --> Space : [\ n]
    Comment --> Comment: [Anything else]

    Operator --> Operator: [is_double_operator]
    Operator --> Space   : [\t|\ n| ]
    Operator --> Number  : [0-9]
    Operator --> Word    : [a-z|_|A-Z]
    Operator --> Comment : [#]
    Operator --> Other   : [Anything else]

    Other --> Space   : [\t|\ n| ]
    Other --> Number  : [0-9]
    Other --> Word    : [a-z|_|A-Z]
    Other --> Comment : [#]
    Other --> Operator: [+|*|/|-|=|(|)|[|]|{|}|<|>]
    Other --> Other   : [Anything else]

```

To further expand the kinds of tokens like adding double operand operators, the logic within the first layer should be altered first to accumulate multiple operators.

~~Currently non alphanumeric characters are only accumulated as 1 character.~~

Now we support double character operators in a pretty trash way. It might eventually be a good idea to come back to this code and work on it. But I think it's more reasonable to work on the actual AST and LLVM frontend.