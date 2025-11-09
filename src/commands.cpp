#include "financial/commands.hpp"

#include <iomanip>

namespace financial {

TimedCommandDecorator::TimedCommandDecorator(std::unique_ptr<ICommand> command, std::ostream& output)
    : command_(std::move(command)), output_(output) {}

void TimedCommandDecorator::execute() {
    auto start = std::chrono::steady_clock::now();
    command_->execute();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    output_ << "Command executed in " << duration.count() << " microseconds\n";
}

AddAccountCommand::AddAccountCommand(AccountFacade& facade,
                                     std::string name,
                                     double balance,
                                     std::ostream& output)
    : facade_(facade), name_(std::move(name)), balance_(balance), output_(output) {}

void AddAccountCommand::execute() {
    auto account = facade_.createAccount(name_, balance_);
    output_ << "Created account #" << account.id() << " (" << account.name() << ") with balance "
            << account.balance() << "\n";
}

AddCategoryCommand::AddCategoryCommand(CategoryFacade& facade,
                                       CategoryType type,
                                       std::string name,
                                       std::ostream& output)
    : facade_(facade), type_(type), name_(std::move(name)), output_(output) {}

void AddCategoryCommand::execute() {
    auto category = facade_.createCategory(type_, name_);
    output_ << "Created category #" << category.id() << " (" << category.name() << ")\n";
}

AddOperationCommand::AddOperationCommand(OperationFacade& facade,
                                         OperationType type,
                                         Identifier accountId,
                                         double amount,
                                         std::chrono::system_clock::time_point date,
                                         std::optional<std::string> description,
                                         Identifier categoryId,
                                         std::ostream& output)
    : facade_(facade),
      type_(type),
      accountId_(accountId),
      amount_(amount),
      date_(date),
      description_(std::move(description)),
      categoryId_(categoryId),
      output_(output) {}

void AddOperationCommand::execute() {
    auto operation =
        facade_.createOperation(type_, accountId_, amount_, date_, description_, categoryId_);
    output_ << "Created operation #" << operation.id() << " amount " << operation.amount()
            << " for account #" << operation.bankAccountId() << "\n";
}

ShowAnalyticsCommand::ShowAnalyticsCommand(AnalyticsFacade& analytics, std::ostream& output)
    : analytics_(analytics), output_(output) {}

void ShowAnalyticsCommand::execute() {
    auto summary = analytics_.balanceSummary();
    output_ << std::fixed << std::setprecision(2);
    output_ << "Income total: " << summary.incomeTotal << "\n";
    output_ << "Expense total: " << summary.expenseTotal << "\n";
    output_ << "Net balance: " << summary.net() << "\n";

    auto breakdown = analytics_.categoryBreakdown();
    output_ << "Category breakdown:" << "\n";
    for (const auto& [category, amount] : breakdown) {
        output_ << " - " << category << ": " << amount << "\n";
    }
}

ImportDataCommand::ImportDataCommand(DataImporter& importer,
                                     std::string path,
                                     DomainFactory& factory,
                                     IRepository<BankAccount>& accountRepository,
                                     IRepository<Category>& categoryRepository,
                                     IOperationRepository& operationRepository,
                                     std::ostream& output)
    : importer_(importer),
      path_(std::move(path)),
      factory_(factory),
      accountRepository_(accountRepository),
      categoryRepository_(categoryRepository),
      operationRepository_(operationRepository),
      output_(output) {}

void ImportDataCommand::execute() {
    auto result = importer_.importFile(path_, factory_);

    for (const auto& account : result.accounts) {
        accountRepository_.add(account);
    }
    for (const auto& category : result.categories) {
        categoryRepository_.add(category);
    }
    for (const auto& operation : result.operations) {
        operationRepository_.add(operation);
    }

    output_ << "Imported " << result.accounts.size() << " accounts, " << result.categories.size()
            << " categories and " << result.operations.size() << " operations from '" << path_ << "'\n";
}

}  // namespace financial
