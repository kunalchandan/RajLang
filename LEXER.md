# Lexer

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