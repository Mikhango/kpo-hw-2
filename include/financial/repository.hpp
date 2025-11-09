#pragma once

#include <map>
#include <optional>
#include <stdexcept>
#include <vector>

#include "financial/domain.hpp"

namespace financial {

template <typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual void add(const T& item) = 0;
    virtual void update(const T& item) = 0;
    virtual void remove(Identifier id) = 0;
    virtual std::optional<T> findById(Identifier id) const = 0;
    virtual std::vector<T> findAll() const = 0;
};

template <typename T>
class InMemoryRepository : public IRepository<T> {
public:
    void add(const T& item) override {
        auto [it, inserted] = storage_.emplace(item.id(), item);
        if (!inserted) {
            throw std::runtime_error("Item with this id already exists");
        }
    }

    void update(const T& item) override {
        auto it = storage_.find(item.id());
        if (it == storage_.end()) {
            throw std::runtime_error("Item not found");
        }
        it->second = item;
    }

    void remove(Identifier id) override { storage_.erase(id); }

    std::optional<T> findById(Identifier id) const override {
        auto it = storage_.find(id);
        if (it == storage_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    std::vector<T> findAll() const override {
        std::vector<T> items;
        items.reserve(storage_.size());
        for (const auto& [_, value] : storage_) {
            items.push_back(value);
        }
        return items;
    }

private:
    std::map<Identifier, T> storage_;
};

class IOperationRepository : public virtual IRepository<Operation> {
public:
    virtual std::vector<Operation> findByAccount(Identifier accountId) const = 0;
};

class OperationRepository : public IOperationRepository {
public:
    void add(const Operation& item) override;
    void update(const Operation& item) override;
    void remove(Identifier id) override;
    std::optional<Operation> findById(Identifier id) const override;
    std::vector<Operation> findAll() const override;
    std::vector<Operation> findByAccount(Identifier accountId) const override;

private:
    std::map<Identifier, Operation> storage_;
};

class CachingOperationRepositoryProxy : public IOperationRepository {
public:
    explicit CachingOperationRepositoryProxy(std::shared_ptr<IOperationRepository> target);

    void add(const Operation& item) override;
    void update(const Operation& item) override;
    void remove(Identifier id) override;
    std::optional<Operation> findById(Identifier id) const override;
    std::vector<Operation> findAll() const override;
    std::vector<Operation> findByAccount(Identifier accountId) const override;

private:
    std::shared_ptr<IOperationRepository> target_;
    mutable std::optional<std::vector<Operation>> cache_;

    void invalidateCache();
};

}  // namespace financial
