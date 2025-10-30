#include "zenin_a_sum_values_by_columns_matrix/mpi/include/ops_mpi.hpp"

#include <mpi.h>
#include <numeric>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstddef>
#include <limits>
#include <type_traits>


#include "util/include/util.hpp"
#include "zenin_a_sum_values_by_columns_matrix/common/include/common.hpp"

namespace zenin_a_sum_values_by_columns_matrix {

ZeninASumValuesByColumnsMatrixMPI::ZeninASumValuesByColumnsMatrixMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType{};
  
}

bool ZeninASumValuesByColumnsMatrixMPI::ValidationImpl() {
  auto& input = GetInput(); 
  bool check_rows = std::get<1>(input).size() % std::get<0>(input) == 0;
  return (std::get<0>(input) > 0) && (!std::get<1>(input).empty()) && (GetOutput().empty()) && check_rows;
  
}

bool ZeninASumValuesByColumnsMatrixMPI::PreProcessingImpl() {
  auto& input = GetInput();
  bool check_rows = std::get<1>(input).size() % std::get<0>(input) == 0; 
  return (GetOutput().empty()) && (std::get<0>(input) > 0) && check_rows && (!std::get<1>(input).empty());
}

bool ZeninASumValuesByColumnsMatrixMPI::RunImpl() {
  auto& input = GetInput();
  
  bool check_rows = std::get<1>(input).size() % std::get<0>(input) == 0; 
  bool testing = (std::get<0>(input) > 0) && (!std::get<1>(input).empty()) && check_rows;
  if (!testing) {
    return false;
  }

  size_t columns = 0;
  std::vector<double> matrix_data;
  int world_size = 0;
  int Rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &Rank);

  if (Rank == 0) {
    columns = std::get<0>(input);
    matrix_data = std::get<1>(input);
  }
  
  MPI_Bcast(&columns, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

  size_t rows = 0;
  if (Rank == 0) {
    rows = matrix_data.size() / columns;
  }

  MPI_Bcast(&rows, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

  size_t base_cols_per_process = columns / world_size;
  size_t remain = columns % world_size;

  size_t start_column = 0;
  size_t end_column = 0;
  size_t cols_this_process = base_cols_per_process;

  if (Rank == world_size - 1) {
    
    start_column = Rank * base_cols_per_process;
    cols_this_process = base_cols_per_process + remain;
    end_column = start_column + cols_this_process;
  } else {
    start_column = Rank * base_cols_per_process;
    end_column = start_column + base_cols_per_process;
    cols_this_process = base_cols_per_process;
  }

  if (Rank != 0) {
    matrix_data.resize(rows * columns);
  }
  MPI_Bcast(matrix_data.data(), static_cast<int>(matrix_data.size()), MPI_DOUBLE, 0, MPI_COMM_WORLD);

  std::vector<double> local_sums(cols_this_process, 0.0);

  for (size_t column = 0; column < cols_this_process; ++column) {
    size_t global_col = start_column + column;
    for (size_t row = 0; row < rows; ++row) {
      local_sums[column] += matrix_data[row * columns + global_col];
    }
  }

  std::vector<double> global_sums;
  if (Rank == 0) {
    global_sums.resize(columns, 0.0);
  }

  std::vector<int> recv_counts(world_size);
  std::vector<int> displacements(world_size);

  if (Rank == 0) {
    for (int i = 0; i < world_size; ++i) {
      if (i == world_size - 1) {
        recv_counts[i] = static_cast<int>(base_cols_per_process + remain);
      } else {
        recv_counts[i] = static_cast<int>(base_cols_per_process);
      }
      
      if (i == 0) {
        displacements[i] = 0;
      } else {
        displacements[i] = displacements[i-1] + recv_counts[i-1];
      }
    }
  }

  MPI_Gatherv(local_sums.data(), static_cast<int>(local_sums.size()), MPI_DOUBLE,
              global_sums.data(), recv_counts.data(), displacements.data(), MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  
  if (Rank == 0) {
    GetOutput() = global_sums;
  }

  return true;


  
}

bool ZeninASumValuesByColumnsMatrixMPI::PostProcessingImpl() {
  return !GetOutput().empty();
 
}

}  // namespace zenin_a_sum_values_by_columns_matrix
