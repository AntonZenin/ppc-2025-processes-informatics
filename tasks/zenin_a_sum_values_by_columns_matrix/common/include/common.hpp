#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

using InType = int;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace zenin_a_sum_values_by_columns_matrix
