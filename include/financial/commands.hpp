#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "financial/facades.hpp"
#include "financial/importers.hpp"
#include "financial/repository.hpp"

namespace financial {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
};

class TimedCommandDecorator : public ICommand {
public:
    TimedCommandDecorator(std::unique_ptr<ICommand> command, std::ostream& output);
    void execute() override;

private:
    std::unique_ptr<ICommand> command_;
    std::ostream& output_;
};

class AddAccountCommand : public ICommand {
public:
    AddAccountCommand(AccountFacade& facade, std::string name, double balance, std::ostream& output);
    void execute() override;

private:
    AccountFacade& facade_;
    std::string name_;
    double balance_;
    std::ostream& output_;
};

class AddCategoryCommand : public ICommand {
public:
    AddCategoryCommand(CategoryFacade& facade, CategoryType type, std::string name, std::ostream& output);
    void execute() override;

private:
    CategoryFacade& facade_;
    CategoryType type_;
    std::string name_;
    std::ostream& output_;
};

class AddOperationCommand : public ICommand {
public:
    AddOperationCommand(OperationFacade& facade,
                        OperationType type,
                        Identifier accountId,
                        double amount,
                        std::chrono::system_clock::time_point date,
                        std::optional<std::string> description,
                        Identifier categoryId,
                        std::ostream& output);
    void execute() override;

private:
    OperationFacade& facade_;
    OperationType type_;
    Identifier accountId_;
    double amount_;
    std::chrono::system_clock::time_point date_;
    std::optional<std::string> description_;
    Identifier categoryId_;
    std::ostream& output_;
};

class ShowAnalyticsCommand : public ICommand {
public:
    ShowAnalyticsCommand(AnalyticsFacade& analytics, std::ostream& output);
    void execute() override;

private:
    AnalyticsFacade& analytics_;
    std::ostream& output_;
};

class ImportDataCommand : public ICommand {
public:
    ImportDataCommand(DataImporter& importer,
                      std::string path,
                      DomainFactory& factory,
                      IRepository<BankAccount>& accountRepository,
                      IRepository<Category>& categoryRepository,
                      IOperationRepository& operationRepository,
                      std::ostream& output);

    void execute() override;

private:
    DataImporter& importer_;
    std::string path_;
    DomainFactory& factory_;
    IRepository<BankAccount>& accountRepository_;
    IRepository<Category>& categoryRepository_;
    IOperationRepository& operationRepository_;
    std::ostream& output_;
};

}  // namespace financial
