#include "zenin_a_sum_values_by_columns_matrix/seq/include/ops_seq.hpp"

#include <numeric>
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
  const auto& input = GetInput();

  int rows = std::get<0>(input);
  int cols = std::get<1>(input);
  const auto& matrix_data = std::get<2>(input);

  if (rows <= 0 || cols <= 0) {
    return false;
  }

  int expected_size = rows * cols;

  if(matrix_data.size() != expected_size) {
    return false;
  }
  return true;
  
}

bool ZeninASumValuesByColumnsMatrixSEQ::PreProcessingImpl() {
  const auto& input = GetInput();
  int rows = std::get<0>(input);
  int cols = std::get<1>(input);
  const auto& matrix_data = std::get<2>(input); 
  return true;
}

bool ZeninASumValuesByColumnsMatrixSEQ::RunImpl() {
  const auto& input = GetInput();
  int rows = std::get<0>(input);
  int cols = std::get<1>(input);
  const auto& matrix_data = std::get<2>(input);

  OutType result(cols, 0); 

  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      int index = row * cols + col;
      result[col] += matrix_data[index];
    }
  }
  GetOutput() = result;
  return true;
}

bool ZeninASumValuesByColumnsMatrixSEQ::PostProcessingImpl() {
  auto& output = GetOutput();

  if(output.empty()) {
    return false;
  }

  std::cout << "Postprocessing: Column sums = [";
  for (size_t i = 0; i < output.size(); ++i) {
    std::cout << output[i];
    if (i < output.size() - 1) std::cout << ", ";
  }
  std::cout << "]" << std::endl;
  return true;
  
}

}  // namespace zenin_a_sum_values_by_columns_matrix
