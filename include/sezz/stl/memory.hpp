#ifndef SEZZ_STL_MEMORY_HPP_
#define SEZZ_STL_MEMORY_HPP_

#include <memory>
#include <unordered_map>
#include <optional>
#include <stdexcept>

namespace sezz {
namespace detail {

class MemoryRuntime {
private:
    template <class Archive>
    friend MemoryRuntime* GetMemoryRuntime(Archive& ar, bool is_serialize);

public:
    MemoryRuntime() {
        unique_order_ = 0;
        shared_order_ = 0;
        serialize_ = false;
        deserialize_ = false;
    }


    std::optional<size_t> UniqueMapGetOrder(void* ptr) {
        auto iter = unique_ptr_map_.find(ptr);
        if (iter == unique_ptr_map_.end()) {
            return {};
        }
        return iter->second;
    }

    std::optional<void*> UniqueMapGetPtr(size_t order) {
        if (order >= unique_ptr_vector_.size()) {
            return {};
        }
        return unique_ptr_vector_[order];
    }

    size_t UniqueMapInsert(void* ptr) {
        auto iter = unique_ptr_map_.find(ptr);
        if (iter != unique_ptr_map_.end()) {
            throw std::runtime_error("Pointer should not exist.");
        }
        unique_ptr_map_.insert(std::make_pair(ptr, unique_order_));
        unique_ptr_vector_.push_back(ptr);
        return unique_order_++;
    }


    std::optional<size_t> SharedMapGetOrder(void* ptr) {
        auto iter = shared_ptr_map_.find(ptr);
        if (iter == shared_ptr_map_.end()) {
            return {};
        }
        return iter->second;
    }

    std::optional<std::shared_ptr<void>> SharedMapGetPtr(size_t order) {
        if (order >= shared_ptr_vector_.size()) {
            return {};
        }
        return shared_ptr_vector_[order];
    }

    size_t SharedMapInsert(std::shared_ptr<void> ptr) {
        auto iter = shared_ptr_map_.find(ptr.get());
        if (iter != shared_ptr_map_.end()) {
            throw std::runtime_error("Pointer should not exist.");
        }
        shared_ptr_map_.insert(std::make_pair(ptr.get(), shared_order_));
        shared_ptr_vector_.push_back(ptr);
        return shared_order_++;
    }

private:
    std::unordered_map<void*, size_t> unique_ptr_map_;
    size_t unique_order_;
    std::vector<void*> unique_ptr_vector_;

    std::unordered_map<void*, size_t> shared_ptr_map_;
    std::vector<std::shared_ptr<void>> shared_ptr_vector_;
    size_t shared_order_;

    bool serialize_;
    bool deserialize_;
};

template <class Archive>
MemoryRuntime* GetMemoryRuntime(Archive& ar, bool is_serialize) {
    auto& runtime = ar.GetMemoryRuntime();
    if (runtime) {

        if (is_serialize) {
            runtime->serialize_ = true;
            if (runtime->deserialize_) {
                ar.ClearMemoryRuntimeContext();
                runtime = nullptr;
            }
        }
        else {
            runtime->deserialize_ = true;
            if (runtime->serialize_) {
                ar.ClearMemoryRuntimeContext();
                runtime = nullptr;
            }
        }
    }
    if (runtime == nullptr) {
        runtime = new MemoryRuntime;
    }
    return runtime;
}

enum class SharedStatus {
    kNullptr,
    kOrder,
    kData
};

} // namespace detail

/*
* detail::SharedStatus
*/
template <class Archive, class T>
    requires std::is_same_v<std::decay_t<T>, detail::SharedStatus>
void Serialize(Archive& ar, T&& val) {
    switch (val) {
    case detail::SharedStatus::kNullptr:
        ar.Save(uint8_t{ 0 });
        break;
    case detail::SharedStatus::kOrder:
        ar.Save(uint8_t{ 1 });
        break;
    case detail::SharedStatus::kData:
        ar.Save(uint8_t{ 2 });
        break;
    default:
        throw std::runtime_error("Invalid detail::SharedStatus enumeration value.");
    }
}
template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires std::is_same_v<std::decay_t<T>, detail::SharedStatus>
T Deserialize(Archive& ar) {
    auto status = ar.Load<uint8_t>();
    switch (status) {
    case 0:
        return detail::SharedStatus::kNullptr;
    case 1:
        return detail::SharedStatus::kOrder;
    case 2:
        return detail::SharedStatus::kData;
    default:
        throw std::runtime_error("Invalid detail::SharedStatus enumeration value.");
    }
}


/*
* std::unique_ptr
*/
template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::unique_ptr<detail::place_t>>
void Serialize(Archive& ar, T&& val) {
    auto ptr = val.get();
    if (!ptr) {
        ar.Save(false);
        return;
    } 
    auto runtime = detail::GetMemoryRuntime(ar, true);
    runtime->UniqueMapInsert(ptr);

    ar.Save(true);
    ar.Save(*val);
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::unique_ptr<detail::place_t>>
T Deserialize(Archive& ar) {
    if (!ar.Load<bool>()) {
        return DecayT{nullptr};
    }
    using DecayRawT = std::decay_t<typename DecayT::element_type>;
    auto unique_ptr = std::make_unique<DecayRawT>(ar.Load<DecayRawT>());

    auto runtime = detail::GetMemoryRuntime(ar, false);
    runtime->UniqueMapInsert(unique_ptr.get());

    return unique_ptr;
}


/*
* std::shared_ptr
*/
template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::shared_ptr<detail::place_t>>
void Serialize(Archive& ar, T&& val) {
    auto ptr = val.get();
    if (!ptr) {
        ar.Save(detail::SharedStatus::kNullptr);
        return;
    }
    auto runtime = detail::GetMemoryRuntime(ar, true);
    auto order = runtime->SharedMapGetOrder(ptr);
    if (!order) {
        ar.Save(detail::SharedStatus::kData);
        auto order = runtime->SharedMapInsert(val);
        ar.Save(*val);
    }
    else {
        ar.Save(detail::SharedStatus::kOrder);
        ar.Save(*order);
    }
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::shared_ptr<detail::place_t>>
T Deserialize(Archive& ar) {
    auto status = ar.Load<detail::SharedStatus>();
    if (status == detail::SharedStatus::kNullptr) {
        return DecayT{ nullptr };
    }

    auto runtime = detail::GetMemoryRuntime(ar, false);
    using DecayRawT = std::decay_t<typename DecayT::element_type>;
    if (status == detail::SharedStatus::kData) {
        auto shared_ptr = std::make_shared<DecayRawT>(ar.Load<DecayRawT>());
        runtime->SharedMapInsert(shared_ptr);
        return shared_ptr;
    }
    else {
        auto order = ar.Load<size_t>();
        auto ptr = runtime->SharedMapGetPtr(order);
        if (!ptr) {
            throw std::runtime_error("Unable to find the shared_ptr that should be constructed.");
        }
        return std::static_pointer_cast<DecayRawT>(*ptr);
    }
}


/*
* std::weak_ptr
*/
template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::weak_ptr<detail::place_t>>
void Serialize(Archive& ar, T&& val) {
    auto shared_ptr = val.lock();
    if (!shared_ptr) {
        ar.Save(false);
        return;
    }
    auto runtime = detail::GetMemoryRuntime(ar, true);
    auto order = runtime->SharedMapGetOrder(shared_ptr.get());
    ar.Save(true);
    ar.Save(*order);
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::weak_ptr<detail::place_t>>
T Deserialize(Archive& ar) {
    if (!ar.Load<bool>()) {
        return DecayT{};
    }
    auto runtime = detail::GetMemoryRuntime(ar, false);
    auto order = ar.Load<size_t>();
    auto ptr = runtime->SharedMapGetPtr(order);
    if (!ptr) {
        throw std::runtime_error("When deserializing weak_ptr, no corresponding shared_ptr was found. Please ensure that shared_ptr is deserialized first.");
    }
    using DecayRawT = std::decay_t<typename DecayT::element_type>;
    return DecayT{ std::static_pointer_cast<DecayRawT>(*ptr) };
}



/*
* raw ptr
*/
template <class Archive, class T, class DecayT = std::decay_t<T>>
    requires std::is_pointer_v<DecayT>
void Serialize(Archive& ar, T&& val) {
    if (!val) {
        ar.Save(false);
        return;
    }
    auto runtime = detail::GetMemoryRuntime(ar, true);
    auto order = runtime->UniqueMapGetOrder(val);
    if (!order) {
        throw std::runtime_error("The raw pointer can only point to object held by unique_ptr.");
    }
    ar.Save(true);
    ar.Save(*order);
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires std::is_pointer_v<DecayT>
T Deserialize(Archive& ar) {
    if (!ar.Load<bool>()) {
        return nullptr;
    }
    auto runtime = detail::GetMemoryRuntime(ar, false);
    auto order = ar.Load<size_t>();
    auto ptr = runtime->UniqueMapGetPtr(order);
    if (!ptr) {
        throw std::runtime_error("The raw pointer can only point to object held by unique_ptr.");
    }
    return reinterpret_cast<DecayT>(*ptr);
}

} // namespace sezz


#endif // SEZZ_STL_MEMORY_HPP_