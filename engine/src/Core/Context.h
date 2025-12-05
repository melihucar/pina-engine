#pragma once

/// Pina Engine - Context (Subsystem Registry)
/// Central registry for all engine subsystems with factory support for user overrides

#include "Export.h"
#include "Memory.h"
#include "Subsystem.h"
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <functional>

namespace Pina {

/// Factory function type for creating subsystems
using SubsystemFactory = std::function<Subsystem*()>;

/// Central context holding all engine subsystems
/// Subsystems are registered by type and can be retrieved via getSubsystem<T>()
/// Users can override default implementations using registerFactory<Interface, Implementation>()
class PINA_API Context {
public:
    Context();
    ~Context();

    // ========================================================================
    // Factory Registration (for user overrides)
    // ========================================================================

    /// Register a factory for a subsystem type
    /// Call before creating subsystems to override defaults
    template<typename Interface, typename Implementation>
    static void registerFactory();

    /// Register a factory function for a subsystem type
    template<typename Interface>
    static void registerFactory(SubsystemFactory factory);

    /// Check if a factory is registered for a type
    template<typename T>
    static bool hasFactory();

    /// Clear all registered factories (mainly for testing)
    static void clearFactories();

    // ========================================================================
    // Subsystem Creation & Registration
    // ========================================================================

    /// Create and register a subsystem using registered factory or default
    /// Returns the created subsystem (owned by Context)
    template<typename T, typename DefaultImpl = T, typename... Args>
    T* createSubsystem(Args&&... args);

    /// Register an existing subsystem instance (Context takes ownership)
    template<typename T>
    void registerSubsystem(T* subsystem);

    /// Register an existing subsystem instance (unique_ptr)
    template<typename T>
    void registerSubsystem(UNIQUE<T> subsystem);

    // ========================================================================
    // Subsystem Access
    // ========================================================================

    /// Get a subsystem by type
    /// Returns nullptr if not registered
    template<typename T>
    T* getSubsystem() const;

    /// Check if a subsystem is registered
    template<typename T>
    bool hasSubsystem() const;

    /// Remove a subsystem (calls shutdown first)
    template<typename T>
    void removeSubsystem();

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /// Initialize all registered subsystems (in registration order)
    void initializeSubsystems();

    /// Update all subsystems (called each frame)
    void updateSubsystems(float deltaTime);

    /// Shutdown all subsystems (in reverse registration order)
    void shutdownSubsystems();

private:
    std::unordered_map<std::type_index, UNIQUE<Subsystem>> m_subsystems;
    std::vector<std::type_index> m_registrationOrder;

    static std::unordered_map<std::type_index, SubsystemFactory> s_factories;
};

// ============================================================================
// Template Implementations
// ============================================================================

template<typename Interface, typename Implementation>
void Context::registerFactory() {
    static_assert(std::is_base_of<Interface, Implementation>::value,
                  "Implementation must derive from Interface");
    static_assert(std::is_base_of<Subsystem, Interface>::value,
                  "Interface must derive from Subsystem");

    s_factories[std::type_index(typeid(Interface))] = []() -> Subsystem* {
        return new Implementation();
    };
}

template<typename Interface>
void Context::registerFactory(SubsystemFactory factory) {
    static_assert(std::is_base_of<Subsystem, Interface>::value,
                  "Interface must derive from Subsystem");
    s_factories[std::type_index(typeid(Interface))] = std::move(factory);
}

template<typename T>
bool Context::hasFactory() {
    return s_factories.find(std::type_index(typeid(T))) != s_factories.end();
}

template<typename T, typename DefaultImpl, typename... Args>
T* Context::createSubsystem(Args&&... args) {
    static_assert(std::is_base_of<Subsystem, T>::value, "T must derive from Subsystem");

    T* subsystem = nullptr;

    // Check for registered factory first
    auto factoryIt = s_factories.find(std::type_index(typeid(T)));
    if (factoryIt != s_factories.end()) {
        // Use registered factory (user override)
        subsystem = static_cast<T*>(factoryIt->second());
    } else {
        // Use default implementation
        subsystem = new DefaultImpl(std::forward<Args>(args)...);
    }

    registerSubsystem<T>(subsystem);
    return subsystem;
}

template<typename T>
void Context::registerSubsystem(T* subsystem) {
    static_assert(std::is_base_of<Subsystem, T>::value, "T must derive from Subsystem");

    std::type_index typeId(typeid(T));
    subsystem->setContext(this);
    m_registrationOrder.push_back(typeId);
    m_subsystems[typeId] = UNIQUE<Subsystem>(subsystem);
}

template<typename T>
void Context::registerSubsystem(UNIQUE<T> subsystem) {
    registerSubsystem<T>(subsystem.release());
}

template<typename T>
T* Context::getSubsystem() const {
    static_assert(std::is_base_of<Subsystem, T>::value, "T must derive from Subsystem");

    auto it = m_subsystems.find(std::type_index(typeid(T)));
    if (it != m_subsystems.end()) {
        return static_cast<T*>(it->second.get());
    }
    return nullptr;
}

template<typename T>
bool Context::hasSubsystem() const {
    return m_subsystems.find(std::type_index(typeid(T))) != m_subsystems.end();
}

template<typename T>
void Context::removeSubsystem() {
    auto typeId = std::type_index(typeid(T));
    auto it = m_subsystems.find(typeId);
    if (it != m_subsystems.end()) {
        it->second->shutdown();
        m_subsystems.erase(it);

        // Remove from registration order
        auto orderIt = std::find(m_registrationOrder.begin(), m_registrationOrder.end(), typeId);
        if (orderIt != m_registrationOrder.end()) {
            m_registrationOrder.erase(orderIt);
        }
    }
}

// ============================================================================
// Subsystem::getSubsystem implementation (requires Context to be complete)
// ============================================================================

template<typename T>
T* Subsystem::getSubsystem() const {
    return m_context ? m_context->getSubsystem<T>() : nullptr;
}

// ============================================================================
// Macro for registering subsystem factories (static initializer)
// ============================================================================

/// Register a subsystem factory before main() runs
/// Usage: REGISTER_SUBSYSTEM_FACTORY(Input, MyCustomInput)
#define REGISTER_SUBSYSTEM_FACTORY(Interface, Implementation) \
    namespace { \
        struct Implementation##_Registrar { \
            Implementation##_Registrar() { \
                ::Pina::Context::registerFactory<Interface, Implementation>(); \
            } \
        } g_##Implementation##_registrar; \
    }

} // namespace Pina
