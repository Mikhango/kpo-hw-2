#pragma once

#include <chrono>
#include <optional>
#include <string>

#include "financial/domain.hpp"

namespace financial {

class DomainFactory {
public:
    DomainFactory();

    BankAccount createAccount(const std::string& name, double initialBalance);
    Category createCategory(CategoryType type, const std::string& name);
    Operation createOperation(OperationType type,
                              Identifier bankAccountId,
                              double amount,
                              std::chrono::system_clock::time_point date,
                              std::optional<std::string> description,
                              Identifier categoryId);

private:
    Identifier nextAccountId_;
    Identifier nextCategoryId_;
    Identifier nextOperationId_;
};

}  // namespace financial
