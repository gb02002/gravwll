# FLAGS

We have a few config levels:

- based
- debug
- perf
- battle-ready

For reg build there no reason for heavy performance enhancing flags(eg pgo)

## Flags we have

### Default

- Gen errors

```text
-Wall -Wextra -Werror -pedantic-errors -Wshadow=compatible-local
```

- Math errors

```text
-Wconversion -Wsign-conversion -Wdouble-promotion 
```

- Safety:

```text
-Wnull-dereference -Wnon-virtual-dtor -Wzero-as-null-pointer-constant

```

- Clean:

```text
-Wuseless-cast -Wredundant-move -Wclass-memaccess
```

- Formatting

```text
-fdiagnostics-color=always
```

### Maybe beneficial

- Math specifics

```text
-Wfloat-equal -ffast-math -fno-math-errno -fno-trapping-math
-ffinite-math-only -fno-signed-zeros -fassociative-math
```

- Optimization

```text
-Wsuggest-attribute=pure/const -Wdisabled-optimization -fopt-info-vec-optimized -Waggressive-loop-optimizations -ftree-vectorize -fopt-info-vec-missed
```

- Memory

```text
-fsanitize=address,undefined -fno-stack-protector -fomit-frame-pointer
-ffunction-sections -fdata-sections
```

- Inline

```text
-finline-functions -finline-limit=1000
```

- Probably too much

```text
-ffast-math
```

- HPC

```text
-ftree-vectorize -march=native

```

## Configs

### Based

```text
-Wall -Wextra -Wpedantic
-Wshadow=compatible-local
-Wconversion -Wsign-conversion -Wdouble-promotion
-Wnull-dereference -Wnon-virtual-dtor -Wzero-as-null-pointer-constant
-Wuseless-cast -Wredundant-move -Wclass-memaccess

```

### Debug

```text
-O0 -g3
-fsanitize=address,undefined
-fno-omit-frame-pointer
```

### Dev

```text
-O2 -g
```

### Battle-ready

```text
-O3 -DNDEBUG
-fno-math-errno
-fno-trapping-math
```

### Perf

```text
-O3 -march=native
-ftree-vectorize
-fno-math-errno
-fno-trapping-math
-ffp-contract=fast
-funsafe-math-optimizations
-fno-math-errno
-fno-trapping-math
-fno-signed-zeros
-fassociative-math
```

## TODO/TOTEST/TOREAD

- [ ] What goes under --fast-math, what can be used
