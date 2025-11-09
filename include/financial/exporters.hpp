#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace financial {

class BankAccount;
class Category;
class Operation;

class DataExportVisitor {
public:
    virtual ~DataExportVisitor() = default;
    virtual void startSection(const std::string& name) { (void)name; }
    virtual void visit(const BankAccount& account) = 0;
    virtual void visit(const Category& category) = 0;
    virtual void visit(const Operation& operation) = 0;
    virtual std::string buildOutput() const = 0;
};

class CsvExportVisitor : public DataExportVisitor {
public:
    void startSection(const std::string& name) override;
    void visit(const BankAccount& account) override;
    void visit(const Category& category) override;
    void visit(const Operation& operation) override;
    std::string buildOutput() const override;

private:
    std::ostringstream output_;
};

class JsonExportVisitor : public DataExportVisitor {
public:
    void startSection(const std::string& name) override;
    void visit(const BankAccount& account) override;
    void visit(const Category& category) override;
    void visit(const Operation& operation) override;
    std::string buildOutput() const override;

private:
    std::string currentSection_;
    std::vector<std::string> accounts_;
    std::vector<std::string> categories_;
    std::vector<std::string> operations_;
};

class YamlExportVisitor : public DataExportVisitor {
public:
    void startSection(const std::string& name) override;
    void visit(const BankAccount& account) override;
    void visit(const Category& category) override;
    void visit(const Operation& operation) override;
    std::string buildOutput() const override;

private:
    std::ostringstream output_;
    std::string currentSection_;
};

class DataExporter {
public:
    explicit DataExporter(DataExportVisitor& visitor);

    template <typename T>
    void exportCollection(const std::vector<T>& items, const std::string& sectionName) {
        visitor_.startSection(sectionName);
        for (const auto& item : items) {
            item.accept(visitor_);
        }
    }

    std::string buildOutput() const;

private:
    DataExportVisitor& visitor_;
};

}  // namespace financial
