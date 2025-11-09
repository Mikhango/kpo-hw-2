#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include "financial/analytics.hpp"
#include "financial/commands.hpp"
#include "financial/di.hpp"
#include "financial/exporters.hpp"
#include "financial/facades.hpp"
#include "financial/importers.hpp"
#include "financial/repository.hpp"

using namespace std::chrono_literals;

namespace {
void writeToFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    file << content;
}

std::string trim(const std::string& value) {
    auto begin = value.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos) {
        return "";
    }
    auto end = value.find_last_not_of(" \t\n\r");
    return value.substr(begin, end - begin + 1);
}

financial::Identifier findCategoryIdByName(const std::vector<financial::Category>& categories,
                                           const std::string& name) {
    for (const auto& category : categories) {
        if (category.name() == name) {
            return category.id();
        }
    }
    throw std::runtime_error("Category not found: " + name);
}

}  // namespace

int main() {
    using namespace financial;

    DomainFactory factory;
    InMemoryRepository<BankAccount> accountRepository;
    InMemoryRepository<Category> categoryRepository;
    auto operationRepositoryImpl = std::make_shared<OperationRepository>();
    auto operationRepository = std::make_shared<CachingOperationRepositoryProxy>(operationRepositoryImpl);

    DIContainer container;
    auto analyticsCore = std::make_shared<AnalyticsService>();
    auto analyticsProxy = std::make_shared<AnalyticsCacheProxy>(analyticsCore);
    container.registerFactory<IAnalyticsService>("analytics", [analyticsProxy]() { return analyticsProxy; });
    auto analyticsService = container.resolve<IAnalyticsService>("analytics");

    AccountFacade accountFacade(factory, accountRepository);
    CategoryFacade categoryFacade(factory, categoryRepository);
    OperationFacade operationFacade(factory, accountRepository, categoryRepository, *operationRepository);
    AnalyticsFacade analyticsFacade(*analyticsService, *operationRepository, categoryRepository);

    std::cout << "=== Financial Accounting Demo ===\n";

    std::cout << "Enter path to data file for import (csv/json/yaml) or leave blank to load sample data:\n> ";
    std::string importPath;
    if (!std::getline(std::cin, importPath)) {
        importPath.clear();
    }
    importPath = trim(importPath);

    bool imported = false;
    if (!importPath.empty()) {
        try {
            auto importer = createImporterForExtension(importPath);
            auto importCommand = std::make_unique<ImportDataCommand>(*importer,
                                                                     importPath,
                                                                     factory,
                                                                     accountRepository,
                                                                     categoryRepository,
                                                                     *operationRepository,
                                                                     std::cout);
            auto timedImport = std::make_unique<TimedCommandDecorator>(std::move(importCommand), std::cout);
            timedImport->execute();
            imported = true;
        } catch (const std::exception& ex) {
            std::cerr << "Failed to import data: " << ex.what() << "\n";
            return 1;
        }
    }

    if (!imported) {
        auto addIncomeCategory = std::make_unique<TimedCommandDecorator>(
            std::make_unique<AddCategoryCommand>(categoryFacade, CategoryType::Income, "Salary", std::cout),
            std::cout);
        addIncomeCategory->execute();

        auto addExpenseCategory = std::make_unique<TimedCommandDecorator>(
            std::make_unique<AddCategoryCommand>(categoryFacade, CategoryType::Expense, "Cafe", std::cout),
            std::cout);
        addExpenseCategory->execute();

        auto addAccount = std::make_unique<TimedCommandDecorator>(
            std::make_unique<AddAccountCommand>(accountFacade, "Main account", 0.0, std::cout), std::cout);
        addAccount->execute();

        auto accounts = accountFacade.listAccounts();
        auto categories = categoryFacade.listCategories();

        auto mainAccountId = accounts.front().id();
        auto salaryCategoryId = findCategoryIdByName(categories, "Salary");
        auto cafeCategoryId = findCategoryIdByName(categories, "Cafe");

        auto addSalaryOperation = std::make_unique<TimedCommandDecorator>(
            std::make_unique<AddOperationCommand>(operationFacade,
                                                  OperationType::Income,
                                                  mainAccountId,
                                                  2500.0,
                                                  std::chrono::system_clock::now(),
                                                  std::string("Monthly salary"),
                                                  salaryCategoryId,
                                                  std::cout),
            std::cout);
        addSalaryOperation->execute();

        auto addCafeExpense = std::make_unique<TimedCommandDecorator>(
            std::make_unique<AddOperationCommand>(operationFacade,
                                                  OperationType::Expense,
                                                  mainAccountId,
                                                  150.0,
                                                  std::chrono::system_clock::now(),
                                                  std::string("Weekend brunch"),
                                                  cafeCategoryId,
                                                  std::cout),
            std::cout);
        addCafeExpense->execute();
    }

    auto showAnalytics = std::make_unique<TimedCommandDecorator>(
        std::make_unique<ShowAnalyticsCommand>(analyticsFacade, std::cout), std::cout);
    showAnalytics->execute();

    std::filesystem::create_directories("data");

    CsvExportVisitor csvVisitor;
    DataExporter csvExporter(csvVisitor);
    csvExporter.exportCollection(accountFacade.listAccounts(), "accounts");
    csvExporter.exportCollection(categoryFacade.listCategories(), "categories");
    csvExporter.exportCollection(operationFacade.listOperations(), "operations");
    writeToFile("data/financial_data.csv", csvExporter.buildOutput());

    JsonExportVisitor jsonVisitor;
    DataExporter jsonExporter(jsonVisitor);
    jsonExporter.exportCollection(accountFacade.listAccounts(), "accounts");
    jsonExporter.exportCollection(categoryFacade.listCategories(), "categories");
    jsonExporter.exportCollection(operationFacade.listOperations(), "operations");
    writeToFile("data/financial_data.json", jsonExporter.buildOutput());

    YamlExportVisitor yamlVisitor;
    DataExporter yamlExporter(yamlVisitor);
    yamlExporter.exportCollection(accountFacade.listAccounts(), "accounts");
    yamlExporter.exportCollection(categoryFacade.listCategories(), "categories");
    yamlExporter.exportCollection(operationFacade.listOperations(), "operations");
    writeToFile("data/financial_data.yaml", yamlExporter.buildOutput());

    std::cout << "Exported data saved to data/financial_data.* files.\n";

    return 0;
}
