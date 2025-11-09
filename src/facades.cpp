#include "financial/facades.hpp"

#include <stdexcept>

namespace financial {

AccountFacade::AccountFacade(DomainFactory& factory, IRepository<BankAccount>& accountsRepository)
    : factory_(factory), repository_(accountsRepository) {}

BankAccount AccountFacade::createAccount(const std::string& name, double initialBalance) {
    auto account = factory_.createAccount(name, initialBalance);
    repository_.add(account);
    return account;
}

void AccountFacade::renameAccount(Identifier id, const std::string& newName) {
    auto existing = repository_.findById(id);
    if (!existing) {
        throw std::runtime_error("Account not found");
    }
    auto account = *existing;
    account.rename(newName);
    repository_.update(account);
}

std::vector<BankAccount> AccountFacade::listAccounts() const { return repository_.findAll(); }

CategoryFacade::CategoryFacade(DomainFactory& factory, IRepository<Category>& categoryRepository)
    : factory_(factory), repository_(categoryRepository) {}

Category CategoryFacade::createCategory(CategoryType type, const std::string& name) {
    auto category = factory_.createCategory(type, name);
    repository_.add(category);
    return category;
}

void CategoryFacade::renameCategory(Identifier id, const std::string& newName) {
    auto existing = repository_.findById(id);
    if (!existing) {
        throw std::runtime_error("Category not found");
    }
    auto category = *existing;
    category.rename(newName);
    repository_.update(category);
}

std::vector<Category> CategoryFacade::listCategories() const { return repository_.findAll(); }

OperationFacade::OperationFacade(DomainFactory& factory,
                                 IRepository<BankAccount>& accountRepository,
                                 IRepository<Category>& categoryRepository,
                                 IOperationRepository& operationRepository)
    : factory_(factory),
      accountRepository_(accountRepository),
      categoryRepository_(categoryRepository),
      operationRepository_(operationRepository) {}

Operation OperationFacade::createOperation(OperationType type,
                                           Identifier accountId,
                                           double amount,
                                           std::chrono::system_clock::time_point date,
                                           std::optional<std::string> description,
                                           Identifier categoryId) {
    auto account = accountRepository_.findById(accountId);
    if (!account) {
        throw std::runtime_error("Account not found");
    }
    if (!categoryRepository_.findById(categoryId)) {
        throw std::runtime_error("Category not found");
    }

    auto operation = factory_.createOperation(type, accountId, amount, date, description, categoryId);

    auto updatedAccount = *account;
    double delta = type == OperationType::Income ? amount : -amount;
    updatedAccount.adjustBalance(delta);
    accountRepository_.update(updatedAccount);

    operationRepository_.add(operation);
    return operation;
}

std::vector<Operation> OperationFacade::listOperations() const {
    return operationRepository_.findAll();
}

AnalyticsFacade::AnalyticsFacade(const IAnalyticsService& analyticsService,
                                 const IOperationRepository& operationRepository,
                                 const IRepository<Category>& categoryRepository)
    : analyticsService_(analyticsService),
      operationRepository_(operationRepository),
      categoryRepository_(categoryRepository) {}

BalanceSummary AnalyticsFacade::balanceSummary() const {
    auto operations = operationRepository_.findAll();
    return analyticsService_.calculateBalanceSummary(operations);
}

CategoryBreakdown AnalyticsFacade::categoryBreakdown() const {
    auto operations = operationRepository_.findAll();
    return analyticsService_.calculateCategoryBreakdown(operations, categoryRepository_);
}

}  // namespace financial
