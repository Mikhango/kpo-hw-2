#include "financial/domain.hpp"

#include "financial/exporters.hpp"

namespace financial {

BankAccount::BankAccount(Identifier id, std::string name, double balance)
    : id_(id), name_(std::move(name)), balance_(balance) {}

Identifier BankAccount::id() const noexcept { return id_; }

const std::string& BankAccount::name() const noexcept { return name_; }

double BankAccount::balance() const noexcept { return balance_; }

void BankAccount::rename(const std::string& newName) { name_ = newName; }

void BankAccount::adjustBalance(double delta) { balance_ += delta; }

void BankAccount::accept(DataExportVisitor& visitor) const { visitor.visit(*this); }

Category::Category(Identifier id, CategoryType type, std::string name)
    : id_(id), type_(type), name_(std::move(name)) {}

Identifier Category::id() const noexcept { return id_; }

CategoryType Category::type() const noexcept { return type_; }

const std::string& Category::name() const noexcept { return name_; }

void Category::rename(const std::string& newName) { name_ = newName; }

void Category::setType(CategoryType type) noexcept { type_ = type; }

void Category::accept(DataExportVisitor& visitor) const { visitor.visit(*this); }

Operation::Operation(Identifier id,
                     OperationType type,
                     Identifier bankAccountId,
                     double amount,
                     std::chrono::system_clock::time_point date,
                     std::optional<std::string> description,
                     Identifier categoryId)
    : id_(id),
      type_(type),
      bankAccountId_(bankAccountId),
      amount_(amount),
      date_(date),
      description_(std::move(description)),
      categoryId_(categoryId) {}

Identifier Operation::id() const noexcept { return id_; }

OperationType Operation::type() const noexcept { return type_; }

Identifier Operation::bankAccountId() const noexcept { return bankAccountId_; }

double Operation::amount() const noexcept { return amount_; }

std::chrono::system_clock::time_point Operation::date() const noexcept { return date_; }

const std::optional<std::string>& Operation::description() const noexcept {
    return description_;
}

Identifier Operation::categoryId() const noexcept { return categoryId_; }

void Operation::accept(DataExportVisitor& visitor) const { visitor.visit(*this); }

}  // namespace financial
