#ifndef VALUEVIEW_H
#define VALUEVIEW_H

#include "Value.h"
#include <vector>

template <verif_mode_t mode>
class ValueView
{
private:
    Value<mode>& m_value;
public:
    ValueView() = delete;
    ValueView(const ValueView& other) = default;
    explicit ValueView(Value<mode>& value) : m_value(value) {};
    ValueView& operator=(const Value<mode>& other);
    // ValueView& operator=(uint64_t val);
    Value<mode>& get() const { return m_value; };
};

template <verif_mode_t mode>
class ValueRef
{
private:
    Value<mode>& m_value;
public:
    ValueRef() = delete;
    ValueRef(const ValueRef& other) = default;
    explicit ValueRef(Value<mode>& value) : m_value(value) {};
    ValueRef& operator=(const Value<mode>& other) { m_value = other; return *this; };
    ValueRef& operator=(bool val) { m_value = val; return *this; };
    ValueRef& operator=(uint8_t val) = delete;
    ValueRef& operator=(uint16_t val) = delete;
    ValueRef& operator=(uint32_t val) = delete;
    ValueRef& operator=(uint64_t val) = delete;
    Value<mode>& get() const { return m_value; };
};

template<verif_mode_t mode>
using ValueVector = std::vector<Value<mode>>;

template <verif_mode_t mode>
class ValueViewVector
{
public:
    // using Value = ValueView<mode>;
    using ValueT = ValueRef<mode>;
private:
    std::vector<ValueT> m_views;
    ValueViewVector(const ValueViewVector& other, size_t up, size_t down);
public:
    ValueViewVector() = default;
    ValueT& operator[](const size_t id) { return m_views.at(id); }
    ValueViewVector operator[](const Range range)
    { return ValueViewVector(*this, range.first, range.second); }
    void push_back(ValueT elem) { m_views.push_back(elem); }
    void pop_back() { m_views.pop_back(); }
    ValueT& back() { m_views.back(); }
    ValueT& front() { m_views.front(); }
    size_t size() const { return m_views.size(); }
    ValueViewVector& operator=(uint64_t vals);
    ValueViewVector& operator=(const ValueVector<mode>& vals);
    bool is_const();
    uint64_t as_uint64_t();
    template <verif_mode_t M>
    friend std::ostream& operator<<(std::ostream& out, const ValueViewVector<M>& view_vector);
};

#include "ValueView.hpp"

#endif // VALUEVIEW_H
