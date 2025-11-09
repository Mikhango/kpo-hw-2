#pragma once

#include <memory>
#include <string>
#include <vector>

#include "financial/analytics.hpp"
#include "financial/domain.hpp"
#include "financial/factory.hpp"
#include "financial/repository.hpp"

namespace financial {

class AccountFacade {
public:
    AccountFacade(DomainFactory& factory, IRepository<BankAccount>& accountsRepository);

    BankAccount createAccount(const std::string& name, double initialBalance);
    void renameAccount(Identifier id, const std::string& newName);
    std::vector<BankAccount> listAccounts() const;

private:
    DomainFactory& factory_;
    IRepository<BankAccount>& repository_;
};

class CategoryFacade {
public:
    CategoryFacade(DomainFactory& factory, IRepository<Category>& categoryRepository);

    Category createCategory(CategoryType type, const std::string& name);
    void renameCategory(Identifier id, const std::string& newName);
    std::vector<Category> listCategories() const;

private:
    DomainFactory& factory_;
    IRepository<Category>& repository_;
};

class OperationFacade {
public:
    OperationFacade(DomainFactory& factory,
                    IRepository<BankAccount>& accountRepository,
                    IRepository<Category>& categoryRepository,
                    IOperationRepository& operationRepository);

    Operation createOperation(OperationType type,
                              Identifier accountId,
                              double amount,
                              std::chrono::system_clock::time_point date,
                              std::optional<std::string> description,
                              Identifier categoryId);

    std::vector<Operation> listOperations() const;

private:
    DomainFactory& factory_;
    IRepository<BankAccount>& accountRepository_;
    IRepository<Category>& categoryRepository_;
    IOperationRepository& operationRepository_;
};

class AnalyticsFacade {
public:
    AnalyticsFacade(const IAnalyticsService& analyticsService,
                    const IOperationRepository& operationRepository,
                    const IRepository<Category>& categoryRepository);

    BalanceSummary balanceSummary() const;
    CategoryBreakdown categoryBreakdown() const;

private:
    const IAnalyticsService& analyticsService_;
    const IOperationRepository& operationRepository_;
    const IRepository<Category>& categoryRepository_;
};

}  // namespace financial
