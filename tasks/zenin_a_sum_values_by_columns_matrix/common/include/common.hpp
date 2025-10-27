#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

using InType = std::tuple<int, int, std::vector<int>>;
using OutType = std::vector<int>;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace zenin_a_sum_values_by_columns_matrix
