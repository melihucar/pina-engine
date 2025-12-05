Review code for Pina Engine style compliance.

## Arguments
- `$ARGUMENTS`: File path or directory to review (optional, defaults to recent changes)

## Instructions

Review the specified files for compliance with Pina Engine coding standards.

### Checklist

1. **Naming Conventions**
   - [ ] Classes use PascalCase
   - [ ] Member variables use m_ prefix
   - [ ] Methods use camelCase
   - [ ] Getters use get/is/has prefix
   - [ ] Lifecycle methods use on* prefix

2. **Header Format**
   - [ ] Uses `#pragma once`
   - [ ] Has `/// Pina Engine - ComponentName` documentation
   - [ ] Classes exported with `PINA_API`
   - [ ] Inside `namespace Pina {}`

3. **Smart Pointers**
   - [ ] Uses `UNIQUE<T>` not `std::unique_ptr<T>`
   - [ ] Uses `MAKE_UNIQUE<T>()` not `std::make_unique<T>()`
   - [ ] Uses `SHARED<T>` not `std::shared_ptr<T>`
   - [ ] Uses `MAKE_SHARED<T>()` not `std::make_shared<T>()`

4. **Virtual Methods**
   - [ ] All overrides use `override` keyword
   - [ ] Virtual destructors properly declared

5. **Code Organization**
   - [ ] Section headers used for grouping (// ======)
   - [ ] Consistent spacing and formatting

6. **Error Handling**
   - [ ] Functions return bool for success/failure
   - [ ] Factories return nullptr on failure
   - [ ] No exceptions used
   - [ ] Errors logged to std::cerr

### Output Format

Report findings as:
```
[file:line] ISSUE: Description
  Suggestion: How to fix
```

Summarize:
- Total issues found
- Issues by category
- Files reviewed
