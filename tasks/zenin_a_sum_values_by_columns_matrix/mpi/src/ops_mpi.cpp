#include "zenin_a_sum_values_by_columns_matrix/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <type_traits>
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
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank != 0) {
    return true;
  }
  size_t columns = std::get<0>(GetInput());
  const std::vector<double> &matrix_data = std::get<1>(GetInput());
  return (columns > 0) && (matrix_data.size() % columns == 0) && (GetOutput().empty());
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
  auto &input = GetInput();

  bool check_rows = std::get<1>(input).size() % std::get<0>(input) == 0;
  bool testing = (std::get<0>(input) > 0) && (!std::get<1>(input).empty()) && check_rows;
  if (!testing) {
    return false;
  }

  int world_size = 0;
  int rank = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  size_t columns = 0;
  std::vector<double> matrix;

  if (rank == 0) {
    columns = std::get<0>(GetInput());
    matrix = std::get<1>(GetInput());

    if (columns == 0 || matrix.empty() || matrix.size() % columns != 0) {
      return false;
    }
  }
  uint64_t columns_u64 = static_cast<uint64_t>(columns);
  MPI_Bcast(&columns_u64, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  columns = static_cast<size_t>(columns_u64);

  uint64_t total_rows_u64 = 0;
  if (rank == 0) {
    total_rows_u64 = static_cast<uint64_t>(matrix.size() / columns);
  }

  MPI_Bcast(&total_rows_u64, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  size_t total_rows = static_cast<size_t>(total_rows_u64);

  size_t total_elems = total_rows * columns;

  std::vector<int> counts(world_size, 0);
  std::vector<int> displacements(world_size, 0);

  size_t elems_per_proc = total_elems / static_cast<size_t>(world_size);
  size_t remainder = total_elems % static_cast<size_t>(world_size);

  int displace = 0;
  for (int i = 0; i < world_size; i++) {
    counts[i] = static_cast<int>(elems_per_proc + (i < remainder ? 1 : 0));
    displacements[i] = displace;
    displace += counts[i];
  }

  std::vector<double> local_buff(static_cast<size_t>(counts[rank]), 0.0);
  if (rank == 0 && matrix.size() != total_elems) {
    return false;
  }

  MPI_Scatterv((rank == 0 ? matrix.data() : nullptr), counts.data(), displacements.data(), MPI_DOUBLE,
               (counts[rank] ? local_buff.data() : nullptr), counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

  std::vector<double> local_sums(columns, 0.0);
  size_t shift = static_cast<size_t>(displacements[rank]) % columns;
  for (size_t i = 0; i < static_cast<size_t>(counts[rank]); ++i) {
    size_t col_index = (i + shift) % columns;
    local_sums[col_index] += local_buff[i];
  }

  GetOutput().assign(columns, 0.0);
  MPI_Allreduce(local_sums.data(), GetOutput().data(), static_cast<int>(columns), MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  return true;
}

bool ZeninASumValuesByColumnsMatrixMPI::PostProcessingImpl() {
  return true;
}

}  // namespace zenin_a_sum_values_by_columns_matrix
