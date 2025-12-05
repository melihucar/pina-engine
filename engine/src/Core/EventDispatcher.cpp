/// Pina Engine - Event Dispatcher Implementation

#include "EventDispatcher.h"

namespace Pina {

// ============================================================================
// Subsystem Lifecycle
// ============================================================================

void EventDispatcher::initialize() {
    // Nothing to initialize
}

void EventDispatcher::update(float deltaTime) {
    (void)deltaTime;
    processQueue();
}

void EventDispatcher::shutdown() {
    clearQueue();
    m_handlers.clear();
}

// ============================================================================
// Subscription
// ============================================================================

bool EventDispatcher::unsubscribe(EventHandle handle) {
    for (auto& [typeIndex, handlers] : m_handlers) {
        auto it = std::find_if(handlers.begin(), handlers.end(),
            [handle](const Handler& h) { return h.handle == handle; });

        if (it != handlers.end()) {
            handlers.erase(it);
            return true;
        }
    }
    return false;
}

// ============================================================================
// Queue Management
// ============================================================================

void EventDispatcher::processQueue() {
    while (!m_eventQueue.empty()) {
        auto event = std::move(m_eventQueue.front());
        m_eventQueue.pop();

        dispatchToHandlers(*event, event->getTypeIndex());
    }
}

void EventDispatcher::clearQueue() {
    while (!m_eventQueue.empty()) {
        m_eventQueue.pop();
    }
}

// ============================================================================
// Internal Helpers
// ============================================================================

void EventDispatcher::sortHandlers(HandlerList& handlers) {
    std::stable_sort(handlers.begin(), handlers.end(),
        [](const Handler& a, const Handler& b) {
            return a.priority < b.priority;  // Lower priority value = higher priority
        });
}

bool EventDispatcher::dispatchToHandlers(Event& event, const std::type_index& typeIndex) {
    auto it = m_handlers.find(typeIndex);
    if (it == m_handlers.end()) {
        return false;
    }

    for (auto& handler : it->second) {
        handler.callback(event);
        if (event.isConsumed()) {
            return true;
        }
    }

    return false;
}

} // namespace Pina
