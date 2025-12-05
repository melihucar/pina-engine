Create a new engine class with proper Pina Engine structure.

## Arguments
- `$ARGUMENTS`: ClassName [directory]
  - ClassName: PascalCase class name (required)
  - directory: Subdirectory under engine/src/ (optional, e.g., "Graphics", "Scene")

## Instructions

1. Parse arguments to get class name and optional directory
2. Create header file: `engine/src/[directory]/ClassName.h`
3. Create source file: `engine/src/[directory]/ClassName.cpp`
4. Use the following templates:

### Header Template (ClassName.h)
```cpp
#pragma once

/// Pina Engine - ClassName

#include "../Core/Export.h"
#include "../Core/Memory.h"

namespace Pina {

class PINA_API ClassName {
public:
    ClassName();
    ~ClassName();

    // ========================================================================
    // Public Interface
    // ========================================================================

private:
    // ========================================================================
    // Private Members
    // ========================================================================
};

} // namespace Pina
```

### Source Template (ClassName.cpp)
```cpp
/// Pina Engine - ClassName Implementation

#include "ClassName.h"

namespace Pina {

// ============================================================================
// Construction
// ============================================================================

ClassName::ClassName() {
}

ClassName::~ClassName() {
}

} // namespace Pina
```

5. Adjust include paths based on directory depth
6. Report the created files
