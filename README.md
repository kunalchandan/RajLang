# Raj Lang
Another basic programming language, targeting the following features.

- Basic signed/unsigned integer and float types
- Arrays
- Maps/dictionaries
- Anonymous functions
- Clear type definitions for everything

I want the language to look something like:
```rust
func main() {
    # Declare some variables
    let xi : i32 = 4;
    let xf : f32 = 4.2;
    # Declare an anonymous function
    let y : func<i32, f32> -> f32 = func (a:i32, b:f32) -> f32 {
        let c : i8 = 2;
        let d : i32 = a as i32;
        let e : f32 = (d as f32) * b;
        return e;
    };
    let z : f32 = y(xi, xf);
    print(z);
}
```

Compile and run with:
```bash
./compile.sh
```

Or use the executable on a target file.

## Testing
Using *Catch* because it's header only.

```bash
git clone https://github.com/kunalchandan/RajLang
cd RajLang/include/
# Testing
git clone https://github.com/log4cplus/Catch
# Command Line parsing
git clone https://github.com/CLIUtils/CLI11
# Enum stuff
git clone https://github.com/Neargye/magic_enum
# Trees
git clone https://github.com/boostorg/boost
cd boost
git submodule update --init
```