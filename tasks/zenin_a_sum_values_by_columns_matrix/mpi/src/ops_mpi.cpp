#include "zenin_a_sum_values_by_columns_matrix/mpi/include/ops_mpi.hpp"

#include <mpi.h>
#include <utility>
#include <cmath>
#include <cstddef>
#include <tuple>
#include <vector>

#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

ZeninASumValuesByColumnsMatrixMPI::ZeninASumValuesByColumnsMatrixMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType{};
}

bool ZeninASumValuesByColumnsMatrixMPI::ValidationImpl() {
  auto &input = GetInput();
  return ((std::get<0>(input)) * std::get<1>(input) == std::get<2>(input).size() && (GetOutput().empty()));
}

bool ZeninASumValuesByColumnsMatrixMPI::PreProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank != 0) {
    return true;
  }
  GetOutput().clear();
  return true;
}

bool ZeninASumValuesByColumnsMatrixMPI::RunImpl() {
  size_t rows = static_cast<int>(std::get<0>(GetInput()));
  size_t cols = static_cast<int>(std::get<1>(GetInput()));
  const std::vector<double> &mat = std::get<2>(GetInput());

  std::vector<double> &global_sum = GetOutput();

  int rank = 0;
  int size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  size_t base = cols / size;
  size_t rest = cols % size;

  size_t my_cols = base + (std::cmp_less(rank, static_cast<int>(rest)) ? 1 : 0); 

  std::vector<int> sendcounts(size);
  std::vector<int> displs(size);
  if (rank == 0) {
    int offset = 0;
    for (size_t proc = 0; proc < (size_t)size; proc++) {
      size_t pc = base + (proc < rest ? 1 : 0);
      sendcounts[proc] = (int)(pc * rows);
      displs[proc] = offset;
      offset += sendcounts[proc];
    }
  }

  std::vector<double> sendbuf;
  if (rank == 0) {
    sendbuf.resize(rows * cols);
    size_t pos = 0;
    for (size_t p = 0; p < (size_t)size; p++) {
      size_t pc_begin = p * base + (p < rest ? p : rest);
      size_t pc_end = pc_begin + (base + (p < rest ? 1 : 0));
      for (size_t col = pc_begin; col < pc_end; col++) {
        for (size_t row = 0; row < rows; row++) {
          sendbuf[pos++] = mat[row * cols + col];
        }
      }
    }
  }
  std::vector<double> local_block(rows * my_cols);
  MPI_Scatterv(sendbuf.data(), sendcounts.data(), displs.data(), MPI_DOUBLE, local_block.data(),
               (int)local_block.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  std::vector<double> local_sum(my_cols, 0.0);
  for (size_t c = 0; c < my_cols; c++) {
    for (size_t r = 0; r < rows; r++) {
      local_sum[c] += local_block[c * rows + r];
    }
  }
  std::vector<int> recvcounts(size), recvdispls(size);
  if (rank == 0) {
    size_t offset = 0;
    for (size_t p = 0; p < (size_t)size; p++) {
      size_t pc = base + (p < rest ? 1 : 0);
      recvcounts[p] = static_cast<int>(pc);
      recvdispls[p] = static_cast<int>(offset);
      offset += pc;
    }
    global_sum.assign(cols, 0.0);
  }
  MPI_Gatherv(local_sum.data(), static_cast<int>(my_cols), MPI_DOUBLE, global_sum.data(), recvcounts.data(), recvdispls.data(),
              MPI_DOUBLE, 0, MPI_COMM_WORLD);
  global_sum.resize(cols);
  MPI_Bcast(global_sum.data(), static_cast<int>(cols), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  return true;
}

bool ZeninASumValuesByColumnsMatrixMPI::PostProcessingImpl() {
  return true;
}

}  // namespace zenin_a_sum_values_by_columns_matrix
