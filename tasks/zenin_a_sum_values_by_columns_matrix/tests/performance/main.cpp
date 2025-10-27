#include <gtest/gtest.h>

#include "util/include/perf_test_util.hpp"
#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"
#include "zenin_a_sum_values_by_columns_matrix/mpi/include/ops_mpi.hpp"
#include "zenin_a_sum_values_by_columns_matrix/seq/include/ops_seq.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

class ZeninASumValuesByMatrixPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 100;
  InType input_data_{};

  void SetUp() override {
    std::vector<std::pair<int, int>> test_sizes = {{10, 15}, {20, 5}, {8, 25}};
    auto [rows, cols] = test_sizes[std::rand() % test_sizes.size()];


    input_data_ = std::make_tuple(rows, cols, std::vector<int>());
    expected_result_.clear();
    expected_result_.resize(matrix_size, 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 50);
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        int value = dis(gen);
        matrix_data.push_back(value);
        expected_result_[j] += value;
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == expected_result_;
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

INSTANTIATE_TEST_SUITE_P(RunModeTests, ZeninASumValuesByMatrixPerfTests, kGtestValues, kPerfTestName);

}  // namespace zenin_a_sum_values_by_columns_matrix
