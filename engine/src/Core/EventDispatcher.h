#pragma once

/// Pina Engine - Event Dispatcher
/// Central subsystem for event subscription and dispatch

#include "Export.h"
#include "Memory.h"
#include "Subsystem.h"
#include "Event.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <queue>
#include <algorithm>

namespace Pina {

// ============================================================================
// Types
// ============================================================================

/// Handle for unsubscribing from events
using EventHandle = uint64_t;

/// Generic event callback type
using EventCallback = std::function<void(Event&)>;

/// Priority levels for event handlers
enum class EventPriority : int32_t {
    Highest = -1000,
    High    = -100,
    Normal  = 0,
    Low     = 100,
    Lowest  = 1000
};

// ============================================================================
// Event Dispatcher Subsystem
// ============================================================================

/// Central event dispatcher subsystem
/// Manages event subscriptions and dispatches events to handlers
class PINA_API EventDispatcher : public Subsystem {
public:
    EventDispatcher() = default;
    ~EventDispatcher() override = default;

    // ========================================================================
    // Subsystem Lifecycle
    // ========================================================================

    void initialize() override;
    void update(float deltaTime) override;
    void shutdown() override;

    // ========================================================================
    // Subscription API
    // ========================================================================

    /// Subscribe to a specific event type
    /// @tparam EventType The event class to listen for
    /// @param callback Function to call when event fires
    /// @param priority Lower values = higher priority (called first)
    /// @return Handle for unsubscribing
    template<typename EventType>
    EventHandle subscribe(std::function<void(EventType&)> callback,
                          EventPriority priority = EventPriority::Normal);

    /// Unsubscribe using handle
    /// @param handle Handle returned from subscribe()
    /// @return true if successfully unsubscribed
    bool unsubscribe(EventHandle handle);

    /// Unsubscribe all handlers for a specific event type
    template<typename EventType>
    void unsubscribeAll();

    // ========================================================================
    // Dispatch API
    // ========================================================================

    /// Dispatch event immediately to all handlers
    /// @param event The event to dispatch
    /// @return true if event was consumed by a handler
    template<typename EventType>
    bool dispatch(EventType& event);

    /// Queue event for deferred dispatch
    /// @param event Event to queue (will be copied)
    template<typename EventType>
    void queue(const EventType& event);

    /// Process all queued events (called automatically in update())
    void processQueue();

    /// Clear the event queue without processing
    void clearQueue();

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Set maximum queue size (oldest events dropped when exceeded)
    void setMaxQueueSize(size_t size) { m_maxQueueSize = size; }

    /// Get current queue size
    size_t getQueueSize() const { return m_eventQueue.size(); }

    /// Get number of handlers for a specific event type
    template<typename EventType>
    size_t getHandlerCount() const;

private:
    /// Internal handler storage
    struct Handler {
        EventHandle handle;
        EventCallback callback;
        int32_t priority;
    };

    using HandlerList = std::vector<Handler>;

    std::unordered_map<std::type_index, HandlerList> m_handlers;
    std::queue<UNIQUE<Event>> m_eventQueue;

    EventHandle m_nextHandle = 1;
    size_t m_maxQueueSize = 1024;

    /// Sort handlers by priority after adding new one
    void sortHandlers(HandlerList& handlers);

    /// Dispatch to handlers for a specific type
    bool dispatchToHandlers(Event& event, const std::type_index& typeIndex);
};

// ============================================================================
// Template Implementations
// ============================================================================

template<typename EventType>
EventHandle EventDispatcher::subscribe(
    std::function<void(EventType&)> callback,
    EventPriority priority)
{
    static_assert(std::is_base_of<Event, EventType>::value,
                  "EventType must derive from Event");

    auto typeIndex = std::type_index(typeid(EventType));
    EventHandle handle = m_nextHandle++;

    // Wrap typed callback in generic callback
    Handler handler;
    handler.handle = handle;
    handler.priority = static_cast<int32_t>(priority);
    handler.callback = [callback](Event& e) {
        callback(static_cast<EventType&>(e));
    };

    m_handlers[typeIndex].push_back(std::move(handler));
    sortHandlers(m_handlers[typeIndex]);

    return handle;
}

template<typename EventType>
void EventDispatcher::unsubscribeAll() {
    auto typeIndex = std::type_index(typeid(EventType));
    m_handlers.erase(typeIndex);
}

template<typename EventType>
bool EventDispatcher::dispatch(EventType& event) {
    static_assert(std::is_base_of<Event, EventType>::value,
                  "EventType must derive from Event");

    return dispatchToHandlers(event, std::type_index(typeid(EventType)));
}

template<typename EventType>
void EventDispatcher::queue(const EventType& event) {
    static_assert(std::is_base_of<Event, EventType>::value,
                  "EventType must derive from Event");

    if (m_eventQueue.size() >= m_maxQueueSize) {
        // Drop oldest event
        m_eventQueue.pop();
    }

    m_eventQueue.push(MAKE_UNIQUE<EventType>(event));
}

template<typename EventType>
size_t EventDispatcher::getHandlerCount() const {
    auto typeIndex = std::type_index(typeid(EventType));
    auto it = m_handlers.find(typeIndex);
    if (it != m_handlers.end()) {
        return it->second.size();
    }
    return 0;
}

} // namespace Pina
