# CONVENTIONS

Mainly taken from Google Style

## File naming

- header: .h
- c++: .cc
- file_names: snake_case

## Namespaces

- namespaces names are in snake_case
- anonymous namespaces for private linking

## Type names

- Classes, structures, aliases: PascalCase
- Interfaces: PascalCase
- gen vars: snake_case
- class members: snake_case_(with underscore)
- func names: snake_case
- constants: kPascalCase
- enums: PascalCase, values: kPascalCase
- macros: ALL_CAPITAL_WITH_UNDERSCORE
- templates: PascalCase. Typed - with T, nonTyped - with N.
- no exceptions after initialization(slow, ha-ha), eg acceptable on warmup while reading config or for memory arena first allocation. Only std:: exceptions
- ptr and ref: * and & with the type: eg `int* variable`, not `int *variable`
