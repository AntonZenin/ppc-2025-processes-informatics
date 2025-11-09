#include "zenin_a_sum_values_by_columns_matrix/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

#include "util/include/util.hpp"
#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

ZeninASumValuesByColumnsMatrixSEQ::ZeninASumValuesByColumnsMatrixSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType{};
}

bool ZeninASumValuesByColumnsMatrixSEQ::ValidationImpl() {
  auto &input = GetInput();
  bool check_rows = std::get<1>(input).size() % std::get<0>(input) == 0;
  return (std::get<0>(input) > 0) && (!std::get<1>(input).empty()) && (GetOutput().empty()) && check_rows;
}

bool ZeninASumValuesByColumnsMatrixSEQ::PreProcessingImpl() {
  auto &input = GetInput();
  bool check_rows = std::get<1>(input).size() % std::get<0>(input) == 0;
  return (GetOutput().empty()) && (std::get<0>(input) > 0) && check_rows && (!std::get<1>(input).empty());
}

bool ZeninASumValuesByColumnsMatrixSEQ::RunImpl() {
  auto &input = GetInput();
  size_t columns = std::get<0>(input);
  const std::vector<double> &matrix_data = std::get<1>(input);
  size_t rows = matrix_data.size() / columns;
  GetOutput().resize(columns, 0.0);
  for (size_t row = 0; row < rows; ++row) {
    for (size_t col = 0; col < columns; ++col) {
      GetOutput()[col] += matrix_data[row * columns + col];
    }
  }
  return true;
}

bool ZeninASumValuesByColumnsMatrixSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace zenin_a_sum_values_by_columns_matrix
