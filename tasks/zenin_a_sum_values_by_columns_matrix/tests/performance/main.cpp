#include <gtest/gtest.h>

#include "util/include/perf_test_util.hpp"
#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"
#include "zenin_a_sum_values_by_columns_matrix/mpi/include/ops_mpi.hpp"
#include "zenin_a_sum_values_by_columns_matrix/seq/include/ops_seq.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

class ZeninASumValuesByMatrixPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    std::string input_filename = "mat_perf.txt";
    std::string Path = ppc::util::GetAbsoluteTaskPath(PPC_ID_zenin_a_sum_values_by_columns_matrix, input_filename);

    std::ifstream in_file_stream(Path);
    if (!in_file_stream.is_open()) {
      throw std::runtime_error("Error while opening file: " + Path);
    }

    size_t rows = 0;
    size_t columns = 0;
    in_file_stream >> rows >> columns;
    std::vector<double> matrix_data;
    matrix_data.reserve(rows * columns);

    double value;
    while (in_file_stream >> value) {
      matrix_data.push_back(value);
    }

    if (matrix_data.size() != rows * columns) {
      throw std::runtime_error("Invalid matrix data");
    }

    input_data_ = std::make_tuple(columns, matrix_data);

    in_file_stream.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    bool result = true;
    size_t columns = std::get<0>(input_data_);
    const std::vector<double> &matrix_data = std::get<1>(input_data_);
    size_t rows = matrix_data.size() / columns;

    if (output_data.size() != columns) {
      result = false;
      return result;
    }

    std::vector<double> expected_sums(columns, 0.0);

    for (size_t row = 0; row < rows; ++row) {
      for (size_t column = 0; column < columns; ++column) {
        expected_sums[column] += matrix_data[row * columns + column];
      }
    }

    for (size_t column = 0; column < columns; ++column) {
      if (std::abs(output_data[column] - expected_sums[column]) > 1e-9) {
        return false;
      }
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(ZeninASumValuesByMatrixPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, ZeninASumValuesByColumnsMatrixMPI, ZeninASumValuesByColumnsMatrixSEQ>(
        PPC_SETTINGS_zenin_a_sum_values_by_columns_matrix);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ZeninASumValuesByMatrixPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(ZeninAPerfTestMatrix, ZeninASumValuesByMatrixPerfTests, kGtestValues, kPerfTestName);

}  // namespace zenin_a_sum_values_by_columns_matrix
