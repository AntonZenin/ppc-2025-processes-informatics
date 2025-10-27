#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"
#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"
#include "zenin_a_sum_values_by_columns_matrix/mpi/include/ops_mpi.hpp"
#include "zenin_a_sum_values_by_columns_matrix/seq/include/ops_seq.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

class ZeninASumValuesByMatrixFunctTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = GetParam();
    int matrix_size = std::get<0>(params);

    int rows, cols;
    if (matrix_size == 3) {
      rows = 3;
      cols = 3;  // 3x3
    } else if (matrix_size == 5) {
      rows = 5, cols = 3;  // 5x3
    } else {
      rows = 2, cols = 7;
    }

    input_data_ = std::make_tuple(rows, cols, std::vector<int>());
    expected_result_.clear();
    expected_result_.resize(cols, 0);

    auto &matrix_data = std::get<2>(input_data_);
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
    return output_data_ == expected_result_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_result_;
};

namespace {

TEST_P(ZeninASumValuesByMatrixFunctTests, SumByColumnsTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {std::make_tuple(3, "3x3 matrix"), std::make_tuple(5, "5x3 matrix"),
                                            std::make_tuple(7, "2x7 matrix")};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<ZeninASumValuesByColumnsMatrixMPI, InType>(
                                               kTestParam, PPC_SETTINGS_zenin_a_sum_values_by_columns_matrix),
                                           ppc::util::AddFuncTask<ZeninASumValuesByColumnsMatrixSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_zenin_a_sum_values_by_columns_matrix));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ZeninASumValuesByMatrixFunctTests::PrintFuncTestName<ZeninASumValuesByMatrixFunctTests>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, ZeninASumValuesByMatrixFunctTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zenin_a_sum_values_by_columns_matrix
