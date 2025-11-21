#pragma once

#include "task/include/task.hpp"
#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

class ZeninASumValuesByColumnsMatrixMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ZeninASumValuesByColumnsMatrixMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void ComputeSendCountsAndDispls(size_t base, size_t rest, size_t rows, int world_size, std::vector<int> &sendcounts,
                                  std::vector<int> &displs);
  void FillSendBuffer(const std::vector<double> &mat, std::vector<double> &sendbuf, size_t rows, size_t cols,
                      size_t base, size_t rest, int world_size);
  void ComputeLocalSum(const std::vector<double> &local_block, std::vector<double> &local_sum, size_t rows,
                       size_t my_cols);
};

}  // namespace zenin_a_sum_values_by_columns_matrix
