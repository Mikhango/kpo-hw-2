#include "financial/analytics.hpp"

#include <functional>

namespace financial {

BalanceSummary AnalyticsService::calculateBalanceSummary(
    const std::vector<Operation>& operations) const {
    BalanceSummary summary;
    for (const auto& op : operations) {
        if (op.type() == OperationType::Income) {
            summary.incomeTotal += op.amount();
        } else {
            summary.expenseTotal += op.amount();
        }
    }
    return summary;
}

CategoryBreakdown AnalyticsService::calculateCategoryBreakdown(
    const std::vector<Operation>& operations,
    const IRepository<Category>& categories) const {
    CategoryBreakdown breakdown;
    auto categoryList = categories.findAll();
    for (const auto& category : categoryList) {
        breakdown[category.name()] = 0.0;
    }
    for (const auto& operation : operations) {
        auto categoryOpt = categories.findById(operation.categoryId());
        if (!categoryOpt) {
            continue;
        }
        const auto& category = *categoryOpt;
        double signedAmount =
            operation.type() == OperationType::Income ? operation.amount() : -operation.amount();
        breakdown[category.name()] += signedAmount;
    }
    return breakdown;
}

AnalyticsCacheProxy::AnalyticsCacheProxy(std::shared_ptr<IAnalyticsService> target)
    : target_(std::move(target)) {}

BalanceSummary AnalyticsCacheProxy::calculateBalanceSummary(
    const std::vector<Operation>& operations) const {
    auto hash = hashOperations(operations);
    if (!cachedSummary_ || hash != lastOperationsHash_) {
        cachedSummary_ = target_->calculateBalanceSummary(operations);
        lastOperationsHash_ = hash;
    }
    return *cachedSummary_;
}

CategoryBreakdown AnalyticsCacheProxy::calculateCategoryBreakdown(
    const std::vector<Operation>& operations,
    const IRepository<Category>& categories) const {
    auto hash = hashOperations(operations);
    if (!cachedBreakdown_ || hash != lastOperationsHash_) {
        cachedBreakdown_ = target_->calculateCategoryBreakdown(operations, categories);
        lastOperationsHash_ = hash;
    }
    return *cachedBreakdown_;
}

std::size_t AnalyticsCacheProxy::hashOperations(const std::vector<Operation>& operations) const {
    std::size_t seed = 0;
    std::hash<int> intHasher;
    for (const auto& op : operations) {
        seed ^= intHasher(op.id()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

}  // namespace financial
