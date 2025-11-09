#include "financial/importers.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace financial {
namespace {
std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(line);
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::chrono::system_clock::time_point parseDate(const std::string& value) {
    std::tm tm{};
    std::istringstream iss(value);
    iss >> std::get_time(&tm, "%Y-%m-%d");
    if (iss.fail()) {
        throw std::runtime_error("Invalid date format: " + value);
    }
    tm.tm_isdst = -1;
    auto time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

std::string trim(const std::string& str) {
    auto begin = str.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(begin, end - begin + 1);
}

enum class CsvSection { None, Accounts, Categories, Operations };

CsvSection parseSection(const std::string& line) {
    auto lower = line;
    for (auto& ch : lower) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    if (lower == "[accounts]") {
        return CsvSection::Accounts;
    }
    if (lower == "[categories]") {
        return CsvSection::Categories;
    }
    if (lower == "[operations]") {
        return CsvSection::Operations;
    }
    return CsvSection::None;
}

}  // namespace

ImportResult CsvDataImporter::parseContent(const std::string& content, DomainFactory& factory) {
    ImportResult result;
    std::unordered_map<Identifier, Identifier> accountIdMap;
    std::unordered_map<Identifier, Identifier> categoryIdMap;

    CsvSection section = CsvSection::None;
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        auto newSection = parseSection(line);
        if (newSection != CsvSection::None) {
            section = newSection;
            continue;
        }

        auto tokens = split(line, ',');
        switch (section) {
            case CsvSection::Accounts: {
                if (tokens.size() < 3) {
                    continue;
                }
                auto account = factory.createAccount(tokens[1], std::stod(tokens[2]));
                accountIdMap[static_cast<Identifier>(std::stoi(tokens[0]))] = account.id();
                result.accounts.push_back(account);
                break;
            }
            case CsvSection::Categories: {
                if (tokens.size() < 3) {
                    continue;
                }
                auto typeToken = tokens[1] == "income" ? CategoryType::Income : CategoryType::Expense;
                auto category = factory.createCategory(typeToken, tokens[2]);
                categoryIdMap[static_cast<Identifier>(std::stoi(tokens[0]))] = category.id();
                result.categories.push_back(category);
                break;
            }
            case CsvSection::Operations: {
                if (tokens.size() < 7) {
                    continue;
                }
                auto typeToken = tokens[1] == "income" ? OperationType::Income : OperationType::Expense;
                auto oldAccountId = static_cast<Identifier>(std::stoi(tokens[2]));
                auto oldCategoryId = static_cast<Identifier>(std::stoi(tokens[6]));
                auto accountId = accountIdMap[oldAccountId];
                auto categoryId = categoryIdMap[oldCategoryId];
                auto amount = std::stod(tokens[3]);
                auto date = parseDate(tokens[4]);
                std::optional<std::string> description;
                if (!tokens[5].empty()) {
                    description = tokens[5];
                }
                auto operation =
                    factory.createOperation(typeToken, accountId, amount, date, description, categoryId);
                result.operations.push_back(operation);
                break;
            }
            default:
                break;
        }
    }

    return result;
}

ImportResult JsonDataImporter::parseContent(const std::string& content, DomainFactory& factory) {
    ImportResult result;
    std::unordered_map<Identifier, Identifier> accountIdMap;
    std::unordered_map<Identifier, Identifier> categoryIdMap;

    auto findArray = [&](const std::string& key) {
        auto pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) {
            return std::string{};
        }
        auto start = content.find('[', pos);
        auto end = content.find(']', start);
        if (start == std::string::npos || end == std::string::npos) {
            return std::string{};
        }
        return content.substr(start + 1, end - start - 1);
    };

    auto parseObjects = [&](const std::string& arrayContent) {
        std::vector<std::string> objects;
        std::string current;
        int depth = 0;
        for (char ch : arrayContent) {
            if (ch == '{') {
                if (depth == 0) {
                    current.clear();
                }
                ++depth;
            }
            if (depth > 0) {
                current.push_back(ch);
            }
            if (ch == '}') {
                --depth;
                if (depth == 0) {
                    objects.push_back(current);
                }
            }
        }
        return objects;
    };

    auto accountsArray = parseObjects(findArray("accounts"));
    for (const auto& object : accountsArray) {
        auto tokens = split(object.substr(1, object.size() - 2), ',');
        Identifier oldId = 0;
        std::string name;
        double balance = 0.0;
        for (const auto& token : tokens) {
            auto parts = split(token, ':');
            if (parts.size() < 2) {
                continue;
            }
            auto key = trim(parts[0]);
            auto value = trim(parts[1]);
            if (key == "\"id\"") {
                oldId = static_cast<Identifier>(std::stoi(value));
            } else if (key == "\"name\"") {
                name = value.substr(1, value.size() - 2);
            } else if (key == "\"balance\"") {
                balance = std::stod(value);
            }
        }
        auto account = factory.createAccount(name, balance);
        accountIdMap[oldId] = account.id();
        result.accounts.push_back(account);
    }

    auto categoriesArray = parseObjects(findArray("categories"));
    for (const auto& object : categoriesArray) {
        auto tokens = split(object.substr(1, object.size() - 2), ',');
        Identifier oldId = 0;
        std::string name;
        CategoryType type = CategoryType::Expense;
        for (const auto& token : tokens) {
            auto parts = split(token, ':');
            if (parts.size() < 2) {
                continue;
            }
            auto key = trim(parts[0]);
            auto value = trim(parts[1]);
            if (key == "\"id\"") {
                oldId = static_cast<Identifier>(std::stoi(value));
            } else if (key == "\"name\"") {
                name = value.substr(1, value.size() - 2);
            } else if (key == "\"type\"") {
                auto typeValue = value.substr(1, value.size() - 2);
                type = typeValue == "income" ? CategoryType::Income : CategoryType::Expense;
            }
        }
        auto category = factory.createCategory(type, name);
        categoryIdMap[oldId] = category.id();
        result.categories.push_back(category);
    }

    auto operationsArray = parseObjects(findArray("operations"));
    for (const auto& object : operationsArray) {
        auto tokens = split(object.substr(1, object.size() - 2), ',');
        OperationType type = OperationType::Expense;
        Identifier accountId{};
        Identifier categoryId{};
        double amount{};
        std::string dateStr;
        std::optional<std::string> description;
        for (const auto& token : tokens) {
            auto parts = split(token, ':');
            if (parts.size() < 2) {
                continue;
            }
            auto key = trim(parts[0]);
            auto value = trim(parts[1]);
            if (key == "\"type\"") {
                auto typeValue = value.substr(1, value.size() - 2);
                type = typeValue == "income" ? OperationType::Income : OperationType::Expense;
            } else if (key == "\"bank_account_id\"") {
                accountId = accountIdMap[static_cast<Identifier>(std::stoi(value))];
            } else if (key == "\"category_id\"") {
                categoryId = categoryIdMap[static_cast<Identifier>(std::stoi(value))];
            } else if (key == "\"amount\"") {
                amount = std::stod(value);
            } else if (key == "\"date\"") {
                dateStr = value.substr(1, value.size() - 2);
            } else if (key == "\"description\"") {
                auto desc = value.substr(1, value.size() - 2);
                if (!desc.empty()) {
                    description = desc;
                }
            }
        }
        auto operation =
            factory.createOperation(type, accountId, amount, parseDate(dateStr), description, categoryId);
        result.operations.push_back(operation);
    }

    return result;
}

ImportResult YamlDataImporter::parseContent(const std::string& content, DomainFactory& factory) {
    ImportResult result;
    std::unordered_map<Identifier, Identifier> accountIdMap;
    std::unordered_map<Identifier, Identifier> categoryIdMap;

    enum class Section { None, Accounts, Categories, Operations };
    Section section = Section::None;

    std::istringstream stream(content);
    std::string line;
    Identifier currentId{};
    std::string name;
    double balance{};
    CategoryType categoryType{};
    OperationType operationType{};
    Identifier bankAccount{};
    Identifier category{};
    double amount{};
    std::string dateStr;
    std::optional<std::string> description;

    auto commitAccount = [&]() {
        auto account = factory.createAccount(name, balance);
        accountIdMap[currentId] = account.id();
        result.accounts.push_back(account);
    };

    auto commitCategory = [&]() {
        auto categoryObj = factory.createCategory(categoryType, name);
        categoryIdMap[currentId] = categoryObj.id();
        result.categories.push_back(categoryObj);
    };

    auto commitOperation = [&]() {
        auto operation = factory.createOperation(
            operationType, accountIdMap[bankAccount], amount, parseDate(dateStr), description, categoryIdMap[category]);
        result.operations.push_back(operation);
    };

    auto flush = [&]() {
        switch (section) {
            case Section::Accounts:
                commitAccount();
                break;
            case Section::Categories:
                commitCategory();
                break;
            case Section::Operations:
                commitOperation();
                break;
            default:
                break;
        }
    };

    bool newItem = false;
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        if (line == "accounts:" || line == "categories:" || line == "operations:") {
            section = line == "accounts:"     ? Section::Accounts
                      : line == "categories:" ? Section::Categories
                                               : Section::Operations;
            continue;
        }
        if (line.rfind("-", 0) == 0) {
            if (newItem) {
                flush();
            }
            newItem = true;
            currentId = 0;
            name.clear();
            balance = 0.0;
            description.reset();
            continue;
        }
        auto parts = split(line, ':');
        if (parts.size() < 2) {
            continue;
        }
        auto key = trim(parts[0]);
        auto value = trim(parts[1]);
        if (!value.empty() && value[0] == '"') {
            value = value.substr(1, value.size() - 2);
        }
        if (key == "id") {
            currentId = static_cast<Identifier>(std::stoi(value));
        } else if (key == "name") {
            name = value;
        } else if (key == "balance") {
            balance = std::stod(value);
        } else if (key == "type") {
            auto typeValue = value;
            if (section == Section::Categories) {
                categoryType = typeValue == "income" ? CategoryType::Income : CategoryType::Expense;
            } else {
                operationType = typeValue == "income" ? OperationType::Income : OperationType::Expense;
            }
        } else if (key == "bank_account_id") {
            bankAccount = static_cast<Identifier>(std::stoi(value));
        } else if (key == "amount") {
            amount = std::stod(value);
        } else if (key == "date") {
            dateStr = value;
        } else if (key == "description") {
            if (!value.empty()) {
                description = value;
            }
        } else if (key == "category_id") {
            category = static_cast<Identifier>(std::stoi(value));
        }
    }

    flush();

    return result;
}

}  // namespace financial
