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
    Other

    Space --> Space   : [\t|\ n| |+|*|/|-|(|)|[|]|{|}]
    Space --> Number  : [0-9]
    Space --> Word    : [a-z|_|A-Z]
    Space --> Comment : [#]
    Space --> Other   : [Anything else]

    Word  --> Space   : [\t|\ n| |+|*|/|-|(|)|[|]|{|}]
    Word  --> Word    : [a-z|_|A-Z|0-9]
    Word  --> Comment : [#]
    Word  --> Other   : [Anything else]

    Number --> Space  : [\t|\ n| |+|*|/|-|(|)|[|]|{|}]
    Number --> Comment: [#]
    Number --> Word   : [a-z|_|A-Z]
    Number --> Number : [0-9|.|e|E]
    Number --> Other  : [Anything else]

    Comment --> Space : [\ n]

    Other --> Space   : [\t|\ n| ]
    Other --> Number  : [0-9]
    Other --> Word    : [a-z|_|A-Z]
    Other --> Comment : [#]
    Other --> Other   : [Anything else]

```

To further expand the kinds of tokens like adding double operand operators, the logic within the first layer should be altered first to accumulate multiple operators. Currently non alphanumeric characters are only accumulated as 1 character.
