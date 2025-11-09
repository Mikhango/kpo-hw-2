#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace financial {

using Identifier = int;

enum class CategoryType { Income, Expense };

enum class OperationType { Income, Expense };

class DataExportVisitor;

class Exportable {
public:
    virtual ~Exportable() = default;
    virtual void accept(DataExportVisitor& visitor) const = 0;
};

class BankAccount : public Exportable {
public:
    BankAccount(Identifier id, std::string name, double balance);

    Identifier id() const noexcept;
    const std::string& name() const noexcept;
    double balance() const noexcept;

    void rename(const std::string& newName);
    void adjustBalance(double delta);

    void accept(DataExportVisitor& visitor) const override;

private:
    Identifier id_;
    std::string name_;
    double balance_;
};

class Category : public Exportable {
public:
    Category(Identifier id, CategoryType type, std::string name);

    Identifier id() const noexcept;
    CategoryType type() const noexcept;
    const std::string& name() const noexcept;

    void rename(const std::string& newName);
    void setType(CategoryType type) noexcept;

    void accept(DataExportVisitor& visitor) const override;

private:
    Identifier id_;
    CategoryType type_;
    std::string name_;
};

class Operation : public Exportable {
public:
    Operation(Identifier id,
              OperationType type,
              Identifier bankAccountId,
              double amount,
              std::chrono::system_clock::time_point date,
              std::optional<std::string> description,
              Identifier categoryId);

    Identifier id() const noexcept;
    OperationType type() const noexcept;
    Identifier bankAccountId() const noexcept;
    double amount() const noexcept;
    std::chrono::system_clock::time_point date() const noexcept;
    const std::optional<std::string>& description() const noexcept;
    Identifier categoryId() const noexcept;

    void accept(DataExportVisitor& visitor) const override;

private:
    Identifier id_;
    OperationType type_;
    Identifier bankAccountId_;
    double amount_;
    std::chrono::system_clock::time_point date_;
    std::optional<std::string> description_;
    Identifier categoryId_;
};

}  // namespace financial
