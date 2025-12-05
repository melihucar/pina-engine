#pragma once

/// Pina Engine - Event System Base Classes
/// Base class and utilities for the event system

#include "Export.h"
#include <cstdint>
#include <typeindex>

namespace Pina {

// ============================================================================
// Event Categories
// ============================================================================

/// Event categories for filtering and grouping
enum class EventCategory : uint16_t {
    None        = 0,
    Input       = 1 << 0,
    Keyboard    = 1 << 1,
    Mouse       = 1 << 2,
    Window      = 1 << 3,
    Application = 1 << 4
};

/// Combine event categories
inline EventCategory operator|(EventCategory a, EventCategory b) {
    return static_cast<EventCategory>(
        static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

/// Check if categories contain a specific flag
inline bool hasCategory(EventCategory cats, EventCategory flag) {
    return (static_cast<uint16_t>(cats) & static_cast<uint16_t>(flag)) != 0;
}

// ============================================================================
// Base Event Class
// ============================================================================

/// Abstract base class for all events
class PINA_API Event {
public:
    virtual ~Event() = default;

    /// Get event type info for dispatch
    virtual std::type_index getTypeIndex() const = 0;

    /// Get event categories for filtering
    virtual EventCategory getCategories() const = 0;

    /// Get event name for debugging
    virtual const char* getName() const = 0;

    /// Check if event has been consumed (stops propagation)
    bool isConsumed() const { return m_consumed; }

    /// Mark event as consumed to stop propagation
    void consume() { m_consumed = true; }

protected:
    bool m_consumed = false;
};

// ============================================================================
// CRTP Helper for Event Types
// ============================================================================

/// CRTP helper for automatic type registration
/// Usage: class MyEvent : public EventBase<MyEvent> { ... };
template<typename Derived>
class EventBase : public Event {
public:
    std::type_index getTypeIndex() const override {
        return std::type_index(typeid(Derived));
    }

    /// Get static type index for subscription
    static std::type_index staticTypeIndex() {
        return std::type_index(typeid(Derived));
    }
};

} // namespace Pina
