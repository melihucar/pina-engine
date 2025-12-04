/// Memory Tests
/// Tests for Core/Memory type aliases

#include <gtest/gtest.h>
#include <Pina.h>

namespace Pina {
namespace Tests {

// Test class for smart pointer tests
class TestObject {
public:
    TestObject() : m_value(0) { s_instanceCount++; }
    explicit TestObject(int value) : m_value(value) { s_instanceCount++; }
    ~TestObject() { s_instanceCount--; }

    int getValue() const { return m_value; }
    void setValue(int value) { m_value = value; }

    static int getInstanceCount() { return s_instanceCount; }
    static void resetInstanceCount() { s_instanceCount = 0; }

private:
    int m_value;
    static int s_instanceCount;
};

int TestObject::s_instanceCount = 0;

// Test UNIQUE alias
TEST(MemoryTest, UniquePointerAlias) {
    TestObject::resetInstanceCount();

    {
        UNIQUE<TestObject> ptr = MAKE_UNIQUE<TestObject>(42);
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(ptr->getValue(), 42);
        EXPECT_EQ(TestObject::getInstanceCount(), 1);
    }

    // Object should be destroyed when unique_ptr goes out of scope
    EXPECT_EQ(TestObject::getInstanceCount(), 0);
}

// Test SHARED alias
TEST(MemoryTest, SharedPointerAlias) {
    TestObject::resetInstanceCount();

    SHARED<TestObject> ptr1 = MAKE_SHARED<TestObject>(100);
    EXPECT_NE(ptr1, nullptr);
    EXPECT_EQ(ptr1->getValue(), 100);
    EXPECT_EQ(TestObject::getInstanceCount(), 1);

    {
        SHARED<TestObject> ptr2 = ptr1;
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2->getValue(), 100);
        EXPECT_EQ(TestObject::getInstanceCount(), 1);
    }

    // Object still alive because ptr1 holds a reference
    EXPECT_EQ(ptr1.use_count(), 1);
    EXPECT_EQ(TestObject::getInstanceCount(), 1);

    ptr1.reset();
    EXPECT_EQ(TestObject::getInstanceCount(), 0);
}

// Test WEAK alias
TEST(MemoryTest, WeakPointerAlias) {
    TestObject::resetInstanceCount();

    WEAK<TestObject> weakPtr;

    {
        SHARED<TestObject> sharedPtr = MAKE_SHARED<TestObject>(200);
        weakPtr = sharedPtr;

        EXPECT_FALSE(weakPtr.expired());

        SHARED<TestObject> lockedPtr = weakPtr.lock();
        EXPECT_NE(lockedPtr, nullptr);
        EXPECT_EQ(lockedPtr->getValue(), 200);
    }

    // Weak pointer should be expired after shared_ptr is destroyed
    EXPECT_TRUE(weakPtr.expired());
    EXPECT_EQ(weakPtr.lock(), nullptr);
}

// Test MAKE_UNIQUE with default constructor
TEST(MemoryTest, MakeUniqueDefaultConstructor) {
    TestObject::resetInstanceCount();

    UNIQUE<TestObject> ptr = MAKE_UNIQUE<TestObject>();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->getValue(), 0);  // Default value
    EXPECT_EQ(TestObject::getInstanceCount(), 1);
}

// Test MAKE_SHARED with default constructor
TEST(MemoryTest, MakeSharedDefaultConstructor) {
    TestObject::resetInstanceCount();

    SHARED<TestObject> ptr = MAKE_SHARED<TestObject>();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->getValue(), 0);  // Default value
    EXPECT_EQ(TestObject::getInstanceCount(), 1);
}

} // namespace Tests
} // namespace Pina
