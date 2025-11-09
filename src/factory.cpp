#include "financial/factory.hpp"

#include <stdexcept>

namespace financial {

namespace {
Identifier nextIdentifier(Identifier& counter) { return counter++; }
}

DomainFactory::DomainFactory() : nextAccountId_(1), nextCategoryId_(1), nextOperationId_(1) {}

BankAccount DomainFactory::createAccount(const std::string& name, double initialBalance) {
    if (name.empty()) {
        throw std::invalid_argument("Account name cannot be empty");
    }
    return BankAccount(nextIdentifier(nextAccountId_), name, initialBalance);
}

Category DomainFactory::createCategory(CategoryType type, const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Category name cannot be empty");
    }
    return Category(nextIdentifier(nextCategoryId_), type, name);
}

Operation DomainFactory::createOperation(OperationType type,
                                         Identifier bankAccountId,
                                         double amount,
                                         std::chrono::system_clock::time_point date,
                                         std::optional<std::string> description,
                                         Identifier categoryId) {
    if (amount <= 0.0) {
        throw std::invalid_argument("Operation amount must be positive");
    }
    return Operation(nextIdentifier(nextOperationId_),
                     type,
                     bankAccountId,
                     amount,
                     date,
                     std::move(description),
                     categoryId);
}

}  // namespace financial
