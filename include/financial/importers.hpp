#pragma once

#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "financial/domain.hpp"
#include "financial/factory.hpp"
#include "financial/repository.hpp"

namespace financial {

struct ImportResult {
    std::vector<BankAccount> accounts;
    std::vector<Category> categories;
    std::vector<Operation> operations;
};

class DataImporter {
public:
    virtual ~DataImporter() = default;

    ImportResult importFile(const std::string& path, DomainFactory& factory) {
        auto content = readFile(path);
        return parseContent(content, factory);
    }

protected:
    virtual ImportResult parseContent(const std::string& content, DomainFactory& factory) = 0;

private:
    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
};

class CsvDataImporter : public DataImporter {
protected:
    ImportResult parseContent(const std::string& content, DomainFactory& factory) override;
};

class JsonDataImporter : public DataImporter {
protected:
    ImportResult parseContent(const std::string& content, DomainFactory& factory) override;
};

class YamlDataImporter : public DataImporter {
protected:
    ImportResult parseContent(const std::string& content, DomainFactory& factory) override;
};

}  // namespace financial
