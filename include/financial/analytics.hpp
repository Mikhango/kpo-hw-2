#pragma once

#include <map>
#include <string>

#include "financial/domain.hpp"
#include "financial/repository.hpp"

namespace financial {

struct BalanceSummary {
    double incomeTotal{};
    double expenseTotal{};
    double net() const { return incomeTotal - expenseTotal; }
};

using CategoryBreakdown = std::map<std::string, double>;

class IAnalyticsService {
public:
    virtual ~IAnalyticsService() = default;
    virtual BalanceSummary calculateBalanceSummary(
        const std::vector<Operation>& operations) const = 0;
    virtual CategoryBreakdown calculateCategoryBreakdown(
        const std::vector<Operation>& operations,
        const IRepository<Category>& categories) const = 0;
};

class AnalyticsService : public IAnalyticsService {
public:
    BalanceSummary calculateBalanceSummary(
        const std::vector<Operation>& operations) const override;
    CategoryBreakdown calculateCategoryBreakdown(
        const std::vector<Operation>& operations,
        const IRepository<Category>& categories) const override;
};

class AnalyticsCacheProxy : public IAnalyticsService {
public:
    explicit AnalyticsCacheProxy(std::shared_ptr<IAnalyticsService> target);

    BalanceSummary calculateBalanceSummary(
        const std::vector<Operation>& operations) const override;
    CategoryBreakdown calculateCategoryBreakdown(
        const std::vector<Operation>& operations,
        const IRepository<Category>& categories) const override;

private:
    std::shared_ptr<IAnalyticsService> target_;
    mutable std::optional<BalanceSummary> cachedSummary_;
    mutable std::optional<CategoryBreakdown> cachedBreakdown_;
    mutable std::size_t lastOperationsHash_{};

    std::size_t hashOperations(const std::vector<Operation>& operations) const;
};

}  // namespace financial
