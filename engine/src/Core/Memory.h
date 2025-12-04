#pragma once

/// Pina Engine - Memory Management
/// Type aliases for smart pointers

#include <memory>

namespace Pina {

/// Smart pointer type aliases
template<typename T>
using UNIQUE = std::unique_ptr<T>;

template<typename T>
using SHARED = std::shared_ptr<T>;

template<typename T>
using WEAK = std::weak_ptr<T>;

/// Factory functions
template<typename T, typename... Args>
UNIQUE<T> MAKE_UNIQUE(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
SHARED<T> MAKE_SHARED(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace Pina
