#include "benchmark/benchmark.h"
#include "ds/storage/particleBlock.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <immintrin.h>
#include <vector>
#include <xmmintrin.h>

// This is a tryout

#define SOFTENER 1e-20
#define G 6.67430 * 10e-11

// DOESNT WORK. CONVERSION OVERHEAD: vinsertps / vcvtss2sd / vmovq.
// helper: fast inverse sqrt double using one float rsqrt + 1 NR iteration in
// double
static inline double fast_inv_sqrt_double(double r2) {
  // handle non-positive / tiny values safely
  if (r2 <= 0.0)
    return 0.0;

  // 1) approximate inv sqrt using single-precision rsqrt
  // cast to float (may under/overflow for extreme ranges but typical N-body r2
  // is safe)
  float r2f = static_cast<float>(r2);
  // _mm_set_ss loads float into xmm, rsqrt gives approximate reciprocal sqrt
  // (float)
  float approx_f = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(r2f)));
  double inv_r = static_cast<double>(approx_f);

  // 2) Newton-Raphson refinement: inv_r <- inv_r * (1.5 - 0.5 * r2 * inv_r *
  // inv_r) This is the standard NR iteration for 1/sqrt
  double half_r2 = 0.5 * r2;
  // One iteration typically yields good double-ish accuracy when starting from
  // float approx
  inv_r = inv_r * (1.5 - half_r2 * inv_r * inv_r);

  // Optionally a second iteration if you need very high accuracy:
  // inv_r = inv_r * (1.5 - half_r2 * inv_r * inv_r);

  return inv_r;
}

// горизонтальная сумма четырёх double в __m256d
static inline double hsum256_pd(__m256d v) {
  // hadd: [a0+a1, a2+a3, a0+a1, a2+a3] (but in 256 form)
  __m256d t = _mm256_hadd_pd(v, v);
  // extract low 128 and high 128
  __m128d lo = _mm256_castpd256_pd128(t);   // contains (a0+a1, a2+a3)
  __m128d hi = _mm256_extractf128_pd(t, 1); // same
  // Add low and hi's low lanes: result = (a0+a1) + (a2+a3)
  __m128d sum = _mm_add_sd(lo, hi);
  return _mm_cvtsd_f64(sum);
}

ParticleBlock get_random_block() {
  srand(1);
  std::vector<Particle> tmp_;
  tmp_.reserve(16);
  for (int i = 0; i < 16; ++i) {
    tmp_.push_back(Particle{
        static_cast<double>(std::rand()) / RAND_MAX,
        static_cast<double>(std::rand()) / RAND_MAX,
        static_cast<double>(std::rand()) / RAND_MAX,
        static_cast<double>(std::rand()) / RAND_MAX,
        static_cast<double>(std::rand()) / RAND_MAX,
        static_cast<double>(std::rand()) / RAND_MAX,
        static_cast<double>(std::rand()) / RAND_MAX,
    });
  }
  return ParticleBlock{10, tmp_};
}

std::array<ParticleBlock::DataBlock, 27> get_27_blocks() {
  std::array<ParticleBlock::DataBlock, 27> data_blocks{};
  for (int i = 0; i < 27; ++i) {
    data_blocks[i] = get_random_block().data_block;
  }
  return data_blocks;
}

inline void local_pairwise(
    ParticleBlock::DataBlock &prime_block,
    std::array<ParticleBlock::DataBlock, 27> &interaction_blocks_array) {

  const int n0 = prime_block.size;
  if (n0 == 0)
    return;

  // Вынесём указатели на сырые массивы один раз (убираем метод-вызовы в горячем
  // цикле)
  double *const x0_arr = const_cast<double *>(prime_block.get_x().data());
  double *const y0_arr = const_cast<double *>(prime_block.get_y().data());
  double *const z0_arr = const_cast<double *>(prime_block.get_z().data());

  double *const ax_arr = prime_block.get_ax().data();
  double *const ay_arr = prime_block.get_ay().data();
  double *const az_arr = prime_block.get_az().data();

  constexpr double Gval = G;        // локальная копия константы
  constexpr double soft = SOFTENER; // локальная копия softener

  // Для каждого соседнего блока
  for (auto &neighbour_block : interaction_blocks_array) {
    const int n1 = neighbour_block.size;
    if (n1 == 0)
      continue;

    // Сырые указатели соседнего блока — вынесем их тоже
    const double *const x1_arr = neighbour_block.get_x().data();
    const double *const y1_arr = neighbour_block.get_y().data();
    const double *const z1_arr = neighbour_block.get_z().data();
    const double *const mass1_arr = neighbour_block.get_mass().data();

    // Для каждого i: аккумулируем ax/ay/az в регистрах и запишем раз в конце
    for (int i = 0; i < n0; ++i) {
      const double x0 = x0_arr[i];
      const double y0 = y0_arr[i];
      const double z0 = z0_arr[i];

      // Загрузили текущие накопленные ускорения в регистры
      double ax_i = ax_arr[i];
      double ay_i = ay_arr[i];
      double az_i = az_arr[i];

      // Перебираем j — в теле цикла только арифметика и чтение соседних
      // массивов
      for (int j = 0; j < n1; ++j) {
        const double dx = x1_arr[j] - x0;
        const double dy = y1_arr[j] - y0;
        const double dz = z1_arr[j] - z0;

        const double r2 = dx * dx + dy * dy + dz * dz + soft;

        const double inv_r =
            1.0 / std::sqrt(r2); // пока оставляем точный вариант
        const double inv_r2 = inv_r * inv_r;
        const double inv_r3 = inv_r2 * inv_r;

        const double common = Gval * mass1_arr[j] * inv_r3;

        ax_i += common * dx;
        ay_i += common * dy;
        az_i += common * dz;
      } // j

      // Записываем накопленные значения назад (один store на компоненту на i)
      ax_arr[i] = ax_i;
      ay_arr[i] = ay_i;
      az_arr[i] = az_i;
    } // i
  } // neighbours
}

inline void local_pairwise_avx2(
    ParticleBlock::DataBlock &prime_block,
    std::array<ParticleBlock::DataBlock, 27> &interaction_blocks_array) {

  const int n0 = prime_block.size;
  if (n0 == 0)
    return;

  // исходные указатели (убираем методы из горячего цикла)
  double *const x0_arr = const_cast<double *>(prime_block.get_x().data());
  double *const y0_arr = const_cast<double *>(prime_block.get_y().data());
  double *const z0_arr = const_cast<double *>(prime_block.get_z().data());

  double *const ax_arr = prime_block.get_ax().data();
  double *const ay_arr = prime_block.get_ay().data();
  double *const az_arr = prime_block.get_az().data();

  constexpr double Gval = G;
  constexpr double soft = SOFTENER;

  // размер шага в j
  const int V = 4; // 4 doubles per __m256d

  for (auto &neighbour_block : interaction_blocks_array) {
    const int n1 = neighbour_block.size;
    if (n1 == 0)
      continue;

    const double *const x1_arr = neighbour_block.get_x().data();
    const double *const y1_arr = neighbour_block.get_y().data();
    const double *const z1_arr = neighbour_block.get_z().data();
    const double *const mass1_arr = neighbour_block.get_mass().data();

    for (int i = 0; i < n0; ++i) {
      const double x0 = x0_arr[i];
      const double y0 = y0_arr[i];
      const double z0 = z0_arr[i];

      // аккумулируем в регистрах
      double ax_i = ax_arr[i];
      double ay_i = ay_arr[i];
      double az_i = az_arr[i];

      int j = 0;
      // векторная часть: по 4 j за раз
      for (; j + (V - 1) < n1; j += V) {
        // загрузки 4 double
        __m256d x1v = _mm256_loadu_pd(x1_arr + j);
        __m256d y1v = _mm256_loadu_pd(y1_arr + j);
        __m256d z1v = _mm256_loadu_pd(z1_arr + j);
        __m256d m1v = _mm256_loadu_pd(mass1_arr + j);

        // dx, dy, dz vectors: (x1 - x0)
        __m256d x0v = _mm256_set1_pd(x0);
        __m256d y0v = _mm256_set1_pd(y0);
        __m256d z0v = _mm256_set1_pd(z0);

        __m256d dxv = _mm256_sub_pd(x1v, x0v);
        __m256d dyv = _mm256_sub_pd(y1v, y0v);
        __m256d dzv = _mm256_sub_pd(z1v, z0v);

        // r2 = dx*dx + dy*dy + dz*dz + soft
        __m256d dx2 = _mm256_mul_pd(dxv, dxv);
        __m256d dy2 = _mm256_mul_pd(dyv, dyv);
        __m256d dz2 = _mm256_mul_pd(dzv, dzv);

        __m256d r2v = _mm256_add_pd(_mm256_add_pd(dx2, dy2), dz2);
        __m256d softv = _mm256_set1_pd(soft);
        r2v = _mm256_add_pd(r2v, softv);

        // inv_r = 1.0 / sqrt(r2)  (vector sqrt + div)
        __m256d onev = _mm256_set1_pd(1.0);
        __m256d rv = _mm256_sqrt_pd(r2v);
        __m256d inv_rv = _mm256_div_pd(onev, rv);

        // inv_r3 = inv_r * inv_r * inv_r
        __m256d inv_r2v = _mm256_mul_pd(inv_rv, inv_rv);
        __m256d inv_r3v = _mm256_mul_pd(inv_r2v, inv_rv);

        // common = G * m1 * inv_r3
        __m256d Gv = _mm256_set1_pd(Gval);
        __m256d commonv = _mm256_mul_pd(Gv, m1v);
        commonv = _mm256_mul_pd(commonv, inv_r3v);

        // contributions: common * dx/dy/dz
        __m256d ax_contrib_v = _mm256_mul_pd(commonv, dxv);
        __m256d ay_contrib_v = _mm256_mul_pd(commonv, dyv);
        __m256d az_contrib_v = _mm256_mul_pd(commonv, dzv);

        // Суммируем 4 lane'а в скаляр и добавляем
        ax_i += hsum256_pd(ax_contrib_v);
        ay_i += hsum256_pd(ay_contrib_v);
        az_i += hsum256_pd(az_contrib_v);
      } // vector j

      // остаток (tail) — скалярно
      for (; j < n1; ++j) {
        const double dx = x1_arr[j] - x0;
        const double dy = y1_arr[j] - y0;
        const double dz = z1_arr[j] - z0;

        const double r2 = dx * dx + dy * dy + dz * dz + soft;
        const double inv_r = 1.0 / std::sqrt(r2);
        // const double inv_r = 1.0 / std::sqrt(r2);
        const double inv_r2 = inv_r * inv_r;
        const double inv_r3 = inv_r2 * inv_r;
        const double common = Gval * mass1_arr[j] * inv_r3;

        ax_i += common * dx;
        ay_i += common * dy;
        az_i += common * dz;
      }

      // записываем обратно один раз
      ax_arr[i] = ax_i;
      ay_arr[i] = ay_i;
      az_arr[i] = az_i;
    } // i
  } // neighbour blocks
}

// This is the fastest version with 9 microsecs
void BM_p2p_interaction_list(benchmark::State &state) {
  auto data_set = get_27_blocks();
  for (auto _ : state) {
    local_pairwise(data_set[0], data_set);
  }
  benchmark::DoNotOptimize(data_set[0]);
}

void BM_p2p_interaction_list_SIMD(benchmark::State &state) {
  auto data_set = get_27_blocks();
  for (auto _ : state) {
    local_pairwise_avx2(data_set[0], data_set);
  }
  benchmark::DoNotOptimize(data_set[0]);
}

// BENCHMARK(BM_p2p_interaction_list);
// BENCHMARK(BM_p2p_interaction_list_SIMD);
//
BENCHMARK(BM_p2p_interaction_list)->Repetitions(3)->DisplayAggregatesOnly(true);
BENCHMARK(BM_p2p_interaction_list_SIMD)
    ->Repetitions(3)
    ->DisplayAggregatesOnly(true);
// BENCHMARK(BM_p2p_interaction_list_dont_compute_itself);
BENCHMARK_MAIN();
