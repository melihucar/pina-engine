#pragma once

/// Pina Engine - Subsystem Base Class
/// Base class for all engine subsystems (Window, Graphics, Input, etc.)

#include "Export.h"

namespace Pina {

class Context;

/// Base class for all engine subsystems
/// Provides lifecycle methods and context access
class PINA_API Subsystem {
public:
    Subsystem() = default;
    virtual ~Subsystem() = default;

    // Non-copyable, non-movable
    Subsystem(const Subsystem&) = delete;
    Subsystem& operator=(const Subsystem&) = delete;
    Subsystem(Subsystem&&) = delete;
    Subsystem& operator=(Subsystem&&) = delete;

    /// Called after all subsystems are registered
    virtual void initialize() {}

    /// Called every frame before application update
    virtual void update(float deltaTime) { (void)deltaTime; }

    /// Called on shutdown, before destruction
    virtual void shutdown() {}

    /// Get the owning context
    Context* getContext() const { return m_context; }

    /// Get another subsystem from context
    template<typename T>
    T* getSubsystem() const;

private:
    friend class Context;
    void setContext(Context* context) { m_context = context; }

    Context* m_context = nullptr;
};

} // namespace Pina
