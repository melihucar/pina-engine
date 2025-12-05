/// Pina Engine - Context Implementation

#include "Context.h"
#include <algorithm>

namespace Pina {

// Static factory registry
std::unordered_map<std::type_index, SubsystemFactory> Context::s_factories;

Context::Context() = default;

Context::~Context() {
    shutdownSubsystems();
}

void Context::clearFactories() {
    s_factories.clear();
}

void Context::initializeSubsystems() {
    for (const auto& typeId : m_registrationOrder) {
        auto it = m_subsystems.find(typeId);
        if (it != m_subsystems.end()) {
            it->second->initialize();
        }
    }
}

void Context::updateSubsystems(float deltaTime) {
    for (const auto& typeId : m_registrationOrder) {
        auto it = m_subsystems.find(typeId);
        if (it != m_subsystems.end()) {
            it->second->update(deltaTime);
        }
    }
}

void Context::shutdownSubsystems() {
    // Shutdown in reverse order
    for (auto it = m_registrationOrder.rbegin(); it != m_registrationOrder.rend(); ++it) {
        auto subsystemIt = m_subsystems.find(*it);
        if (subsystemIt != m_subsystems.end()) {
            subsystemIt->second->shutdown();
        }
    }
    m_subsystems.clear();
    m_registrationOrder.clear();
}

} // namespace Pina
