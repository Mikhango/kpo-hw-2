#include "financial/exporters.hpp"

#include <iomanip>
#include <sstream>

#include "financial/domain.hpp"

namespace financial {
namespace {
std::string escapeQuotes(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (const auto ch : value) {
        if (ch == '"') {
            result.push_back('\\');
        }
        result.push_back(ch);
    }
    return result;
}

std::string categoryTypeToString(CategoryType type) {
    return type == CategoryType::Income ? "income" : "expense";
}

std::string operationTypeToString(OperationType type) {
    return type == OperationType::Income ? "income" : "expense";
}

std::string formatTime(std::chrono::system_clock::time_point tp) {
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}
}  // namespace

void CsvExportVisitor::startSection(const std::string& name) {
    output_ << "[" << std::uppercase << name << "]\n";
}

void CsvExportVisitor::visit(const BankAccount& account) {
    output_ << account.id() << "," << escapeQuotes(account.name()) << "," << account.balance()
            << "\n";
}

void CsvExportVisitor::visit(const Category& category) {
    output_ << category.id() << "," << categoryTypeToString(category.type()) << ","
            << escapeQuotes(category.name()) << "\n";
}

void CsvExportVisitor::visit(const Operation& operation) {
    output_ << operation.id() << "," << operationTypeToString(operation.type()) << ","
            << operation.bankAccountId() << "," << operation.amount() << ","
            << formatTime(operation.date()) << ","
            << escapeQuotes(operation.description().value_or("")) << ","
            << operation.categoryId() << "\n";
}

std::string CsvExportVisitor::buildOutput() const { return output_.str(); }

void JsonExportVisitor::startSection(const std::string& name) { currentSection_ = name; }

void JsonExportVisitor::visit(const BankAccount& account) {
    std::ostringstream oss;
    oss << "{\"id\":" << account.id() << ",\"name\":\"" << escapeQuotes(account.name())
        << "\",\"balance\":" << account.balance() << "}";
    accounts_.push_back(oss.str());
}

void JsonExportVisitor::visit(const Category& category) {
    std::ostringstream oss;
    oss << "{\"id\":" << category.id() << ",\"type\":\""
        << categoryTypeToString(category.type()) << "\",\"name\":\""
        << escapeQuotes(category.name()) << "\"}";
    categories_.push_back(oss.str());
}

void JsonExportVisitor::visit(const Operation& operation) {
    std::ostringstream oss;
    oss << "{\"id\":" << operation.id() << ",\"type\":\""
        << operationTypeToString(operation.type()) << "\",\"bank_account_id\":"
        << operation.bankAccountId() << ",\"amount\":" << operation.amount()
        << ",\"date\":\"" << formatTime(operation.date())
        << "\",\"description\":\"" << escapeQuotes(operation.description().value_or(""))
        << "\",\"category_id\":" << operation.categoryId() << "}";
    operations_.push_back(oss.str());
}

std::string JsonExportVisitor::buildOutput() const {
    auto join = [](const std::vector<std::string>& values) {
        std::ostringstream inner;
        for (std::size_t i = 0; i < values.size(); ++i) {
            inner << values[i];
            if (i + 1 < values.size()) {
                inner << ",";
            }
        }
        return inner.str();
    };
    std::ostringstream oss;
    oss << "{\"accounts\":[" << join(accounts_) << "],\"categories\":[" << join(categories_)
        << "],\"operations\":[" << join(operations_) << "]}";
    return oss.str();
}

void YamlExportVisitor::startSection(const std::string& name) {
    currentSection_ = name;
    output_ << name << ":\n";
}

void YamlExportVisitor::visit(const BankAccount& account) {
    output_ << "  - id: " << account.id() << "\n    name: " << account.name()
            << "\n    balance: " << account.balance() << "\n";
}

void YamlExportVisitor::visit(const Category& category) {
    output_ << "  - id: " << category.id() << "\n    type: " << categoryTypeToString(category.type())
            << "\n    name: " << category.name() << "\n";
}

void YamlExportVisitor::visit(const Operation& operation) {
    output_ << "  - id: " << operation.id() << "\n    type: "
            << operationTypeToString(operation.type())
            << "\n    bank_account_id: " << operation.bankAccountId()
            << "\n    amount: " << operation.amount() << "\n    date: " << formatTime(operation.date())
            << "\n    description: " << operation.description().value_or("")
            << "\n    category_id: " << operation.categoryId() << "\n";
}

std::string YamlExportVisitor::buildOutput() const { return output_.str(); }

DataExporter::DataExporter(DataExportVisitor& visitor) : visitor_(visitor) {}

std::string DataExporter::buildOutput() const { return visitor_.buildOutput(); }

}  // namespace financial
