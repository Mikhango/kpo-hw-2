#include "financial/repository.hpp"

namespace financial {

void OperationRepository::add(const Operation& item) {
    auto [it, inserted] = storage_.emplace(item.id(), item);
    if (!inserted) {
        throw std::runtime_error("Operation with this id already exists");
    }
}

void OperationRepository::update(const Operation& item) {
    auto it = storage_.find(item.id());
    if (it == storage_.end()) {
        throw std::runtime_error("Operation not found");
    }
    it->second = item;
}

void OperationRepository::remove(Identifier id) { storage_.erase(id); }

std::optional<Operation> OperationRepository::findById(Identifier id) const {
    auto it = storage_.find(id);
    if (it == storage_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<Operation> OperationRepository::findAll() const {
    std::vector<Operation> operations;
    operations.reserve(storage_.size());
    for (const auto& [_, value] : storage_) {
        operations.push_back(value);
    }
    return operations;
}

std::vector<Operation> OperationRepository::findByAccount(Identifier accountId) const {
    std::vector<Operation> result;
    for (const auto& [_, operation] : storage_) {
        if (operation.bankAccountId() == accountId) {
            result.push_back(operation);
        }
    }
    return result;
}

CachingOperationRepositoryProxy::CachingOperationRepositoryProxy(
    std::shared_ptr<IOperationRepository> target)
    : target_(std::move(target)) {}

void CachingOperationRepositoryProxy::add(const Operation& item) {
    target_->add(item);
    invalidateCache();
}

void CachingOperationRepositoryProxy::update(const Operation& item) {
    target_->update(item);
    invalidateCache();
}

void CachingOperationRepositoryProxy::remove(Identifier id) {
    target_->remove(id);
    invalidateCache();
}

std::optional<Operation> CachingOperationRepositoryProxy::findById(Identifier id) const {
    if (cache_) {
        for (const auto& op : *cache_) {
            if (op.id() == id) {
                return op;
            }
        }
    }
    return target_->findById(id);
}

std::vector<Operation> CachingOperationRepositoryProxy::findAll() const {
    if (!cache_) {
        cache_ = target_->findAll();
    }
    return *cache_;
}

std::vector<Operation> CachingOperationRepositoryProxy::findByAccount(Identifier accountId) const {
    std::vector<Operation> result;
    for (const auto& operation : findAll()) {
        if (operation.bankAccountId() == accountId) {
            result.push_back(operation);
        }
    }
    return result;
}

void CachingOperationRepositoryProxy::invalidateCache() { cache_.reset(); }

}  // namespace financial
