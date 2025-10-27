#include "zenin_a_sum_values_by_columns_matrix/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <numeric>
#include <vector>

#include "util/include/util.hpp"
#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

ZeninASumValuesByColumnsMatrixMPI::ZeninASumValuesByColumnsMatrixMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType{};
}

bool ZeninASumValuesByColumnsMatrixMPI::ValidationImpl() {
  const auto &input = GetInput();
  int rows = std::get<0>(input);
  int cols = std::get<1>(input);
  const auto &matrix_data = std::get<2>(input);

  int initialized;
  MPI_Initialized(&initialized);
  if (!initialized) {
    MPI_Init(nullptr, nullptr);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &world_size_);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank_);

  if (rows <= 0 || cols <= 0) {
    if (world_rank_ == 0) {
      std::cout << "Error";
    }
    return false
  }

  int expected_size = rows * cols;
  if (matrix_data.size() != expected_size) {
    if (world_rank_ == 0) {
      std::cout << "Error";
    }
    return false;
  }
  if (world_size_ > cols) {
    if (world_rank_ == 0) {
      std::cout << "Error";
    }
  }

  if (world_rank_ == 0) {
    std::cout << "Validation passed";
  }
  return true;
}

bool ZeninASumValuesByColumnsMatrixMPI::PreProcessingImpl() {
  const auto &input = GetInput();
  int rows = std::get<0>(input);
  int cols = std::get<1>(input);
  const auto &matrix_data = std::get<2>(input);

  int matrix_info[2] = {rows, cols};
  MPI_Bcast(matrix_info, 2, MPI_INT, 0, MPI_COMM_WORLD);

  if (world_rank_ == 0) {
    int base_cols = cols / world_size_;
    int remainder = cols % world_size_;

    for (int i = 0; i < world_size_; ++i) {
      int proc_cols = (i < remainder) ? base_cols + 1 : base_cols;
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  return true;
}

bool ZeninASumValuesByColumnsMatrixMPI::RunImpl() {
  const auto &input = GetInput();
  int rows = std::get<0>(input);
  int cols = std::get<1>(input);
  const auto &matrix_data = std::get<2>(input);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size_);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank_);

  int base_cols_per_process = cols / world_size_;
  int reminder = cols % world_size_;

  int my_cols_start, my_cols_count;
  if (world_rank_ < remainder) {
    my_cols_count = base_cols_per_process + 1;
    my_cols_start = world_rank_ * my_cols_count;
  } else {
    my_cols_count = base_cols_per_process;
    my_cols_start = remainder * (base_cols_per_process + 1) + (world_rank - remainder) * base_cols_per_process;
  }

  std::vector<int> my_column_sums(my_cols_count, 0);
  for (int col = 0; col < my_cols_count; ++col) {
    int global_col = my_cols_start + col;
    for (int row = 0; row < rows; ++row) {
      int index = row * cols + global_col;
      my_column_sums[col] += matrix_data[index];
    }
  }

  if (world_rank == 0) {
    OutType result(cols, 0);
    for (int i = 0; i < my_cols_count; ++i) {
      result[my_cols_start + i] = my_column_sums[i];
    }
    for (int proc = 1; proc < world_size_; ++proc) {
      int proc_cols_count, proc_cols_start;
      if (proc < remainder) {
        proc_cols_count = base_cols_per_process + 1;
        proc_cols_start = proc * proc_cols_count;
      } else {
        proc_cols_count = base_cols_per_process;
        proc_cols_start = remainder * (base_cols_per_process + 1) + (proc - remainder) * base_cols_per_process;
      }
      std::vector<int> proc_results(proc_cols_count);
      MPI_Recv(proc_results.data(), proc_cols_count, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (int i = 0; i < proc_cols_count; ++i) {
        result[proc_cols_start + i] = proc_results[i];
      }
    }
    GetOutput() = result;
  } else {
    MPI_Send(my_column_sums.data(), my_cols_count, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  return true;
}

bool ZeninASumValuesByColumnsMatrixMPI::PostProcessingImpl() {
  if (world_rank == 0) {
    auto &output = GetOutput();
    if (output.empty()) {
      std::cout << "Error";
      return false;
    }
    std::cout << "Postprocessing: Column sums = [";
    for (size_t i = 0; i < std::min(output.size(), size_t(10)); ++i) {
      std::cout << output[i];
      if (i < output.size() - 1 && i < 9) {
        std::cout << ", ";
      }
    }
    if (output.size() > 10) {
      std::cout << ", ... (" << output.size() - 10 << " more)";
    }
    std::cout << "]" << std::endl;
  }

  int finalized;
  MPI_Finalized(&finalized);
  if (!finalized) {
    MPI_Finalize();
  }
  return true;
}

}  // namespace zenin_a_sum_values_by_columns_matrix
