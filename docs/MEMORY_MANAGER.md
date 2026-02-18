# Memory Manager for Octree: Toward Predictable Neighbor Data Layout

## Context and Motivation

In the Fast Multipole Method (FMM), the most frequent and expensive operation is the **particle‑to‑particle (P2P)** interaction between nearby leaves. To achieve high performance, we need to iterate over all 26 neighbors of a leaf and access the particle data stored in their associated blocks **with minimal overhead**.

The ideal traversal would:

- **Avoid pointer chasing** – instead of dereferencing a pointer stored in the leaf node, we want to compute the address of the neighbor’s block directly: `base + neighbor_index * block_size`.
- **Guarantee spatial locality** – the blocks of neighboring leaves should reside in memory at **fixed, predictable offsets** from each other. For example, if a leaf corresponds to block index `i`, its neighbor in the +x direction should always be at index `i + dx`, where `dx` is a compile‑time constant (or a small set of constants for the 26 directions). This would allow the traversal code to prefetch and stream data without consulting the tree structure at all.
- **Remain consistent** under dynamic tree modifications (split, merge, squeeze) – the mapping from leaf to block index must be stable or at least change in a way that preserves the fixed‑offset property.

Our target scale is **1‑3×10⁵ particles**, with **16 particles per block** (a small block size chosen for cache efficiency, but subject to change). This means the number of leaf blocks is on the order of 10⁴ – manageable for experimenting with different allocation strategies.

## Current Design and Its Shortcomings

Currently, the octree nodes are stored in a flat array. Each leaf contains a **pointer** (or reference) to a separate block that holds the particle data in an SoA layout. The blocks are managed by a low‑level pool allocator.

**Problems:**

1. **Pointer dereference in the hot path** – Each neighbor access requires following a pointer, which can cause cache misses and prevents prefetching.
2. **No guaranteed neighbor offset** – Even if we replace pointers with indices, the pool is not organised spatially. Neighboring leaves may have block indices that are arbitrarily far apart, leading to random memory access patterns.
3. **Stability under memory reorganization** – When blocks are moved during defragmentation or tree updates, the index stored in the leaf becomes stale. Maintaining consistency without adding overhead to every access is non‑trivial.

## Ideal Solution: A Spatially‑Aware Block Allocator

We envision a memory manager that:

- Assigns **block indices** to leaves in a way that mirrors the spatial order of the leaves themselves (e.g., Morton order). If leaves are stored in Morton order, then spatially close leaves have close indices.
- Ensures that **the 26 neighbors** of a leaf with index `i` have indices that can be expressed as `i + offset_k` where `offset_k` is a **known constant** (or a small lookup table that is the same for all leaves). This would allow a tight loop:

   ```cpp
   for (int dir = 0; dir < 26; ++dir) {
       Block* nb = pool_base + (my_idx + neighbor_offset[dir]) * block_size;
       process(nb);
   }
   ```

   No tree traversal, no pointer indirection – just arithmetic.

- Handles **dynamic changes** (split, merge) without breaking this property. When a leaf is split into 8 children, new blocks must be allocated such that they also obey the fixed‑offset relations relative to each other and to existing neighbors.

## Possible Approaches

### 1. Full Morton‑Ordered Pool with Pre‑computed Gaps

- Pre‑allocate a large virtual address space and assign each potential leaf (based on maximum tree depth) a fixed slot. The slot index is derived from the leaf’s Morton code.
- Neighbor offsets can be pre‑computed from the Morton code arithmetic (there are known formulas for finding neighbors in a Z‑order curve). However, these offsets are not constant across the whole tree – they depend on the relative positions of the nodes (e.g., the offset to the +x neighbor varies depending on whether you are on a boundary or not). To get truly constant offsets, we would need to enforce a regular grid (i.e., all leaves at the same depth, no adaptivity). That conflicts with our need for adaptivity (FMM requires variable‑depth leaves).

### 2. Hierarchical Offset Tables (HOT)

- For each leaf depth, pre‑compute a small table of 26 offsets that are valid for all leaves at that depth, assuming a **regular refinement** at that depth. This would work if the tree is **balanced** – i.e., all leaves are at the same depth, or at least the majority are, and we can pad with dummy blocks for missing leaves. Then neighbor indices can be computed as `base_index + depth_offset[dir] + local_adjustment` (where local_adjustment handles boundary conditions).
- When a leaf is split, its children are placed in consecutive slots (by Morton order) and the parent’s block is either freed or reused. The offsets for the children relative to each other are fixed (e.g., the eight sub‑blocks have known offsets from the parent’s index if we allocate them in a contiguous range).

### 3. Two‑Level Indexing: Stable Leaf IDs + Dynamic Mapping

- Leaves hold a **stable ID** (e.g., a 64‑bit integer derived from the Morton code). An indirection table maps ID → current block index. The block pool is periodically reordered (defragmented) to improve spatial locality, and the mapping table is updated accordingly.
- During P2P traversal, we still need to get the neighbor’s block index. We could store in each leaf a pre‑computed list of 26 neighbor IDs (computed once when the tree is built/updated). Then the traversal becomes: for each neighbor ID, look up its block index in the mapping table. This adds one level of indirection (the table lookup), but the table can be small and cache‑friendly if we store it in a flat array indexed by ID.
- This approach decouples the logical connectivity (neighbor relations) from physical layout. The physical pool can be reorganised arbitrarily as long as the mapping table is updated atomically. The cost is the extra indirection and the need to maintain the neighbor ID list.

### 4. Hybrid: Fixed Offsets for Regular Regions, Fallback for Irregular

- Use approach [#2](#2-hierarchical-offset-tables-hot) for most leaves that are in a well‑behaved region (deep enough that the tree is nearly regular), and handle boundary/adaptivity cases with a slower path that consults the tree. This could give the best of both worlds, although implementation complexity and dispatch overhead must be taken into account.

## Open Questions and Design Challenges

1. **Can we achieve constant offsets in an adaptive octree, and moreover, will it payoff?**  
   Even with Morton ordering, the offset to a neighbor in, say, the +x direction depends on the relative depth of the two leaves. If one leaf is deeper than its neighbor, the neighbor might actually be a group of smaller cells, and the offset formula becomes more complex. Is there a way to always place blocks so that all 26 possible neighbor blocks (including those at different depths) have fixed offsets? Or must we accept that sometimes we need to fall back to a slower method?

2. **How to handle splits and merges atomically?**  
   When a leaf splits, we need to allocate 8 new blocks and possibly free the parent’s block. The new blocks must be placed such that their indices are contiguous (or at least have known offsets) to preserve the fixed‑offset property. If the pool is heavily fragmented, this might not be possible without a global reorganisation. Is it acceptable to sometimes “steal” space from neighbouring regions, or do we need a compaction mechanism?

3. **Concurrency: multiple threads reading while a reorganisation happens**  
   If we ever need to move blocks (defragmentation, reindexing), how do we ensure that ongoing traversals see a consistent view? Techniques like RCU or generation counters could be used, but they add complexity.

4. **How to compute neighbor IDs quickly?**  
   If we go with the stable ID + mapping table approach, we still need a way to get the 26 neighbor IDs for a given leaf. This can be done once when the tree is built/updated, and stored in the leaf (26 64‑bit integers). That’s 26×8 = 208 bytes per leaf – maybe acceptable if the number of leaves is modest. Alternatively, we could compute them on the fly from the Morton code (using bit manipulations) if that is faster than memory access. This is an empirical question.

## Formal Optimization Model

Choosing a memory management strategy for the octree can be viewed as a multi‑objective optimization problem. While an exact analytical solution in real time is infeasible, a formal model helps identify the key metrics and trade‑offs, and serves as a foundation for heuristics and performance monitoring.

### Measurable System Parameters

Let us denote:

- \(N_b\) – number of leaf blocks (leaves). Expected range: \(10^4\)–\(2\times 10^4\).
- \(B\) – block size in bytes (including SoA storage for 16 particles). Currently, with 16 particles and 3*(coordinates + velocity + acceleration + jitter) (all double) + visual_id + mass, roughly \(16 \times 14 \times 8 = 1792\) bytes.
- \(P = N_b \cdot B\) – total particle data volume.
- \(M\) – number of neighbor accesses processed in one P2P step. For each leaf up to 26 neighbors, total \(M \approx 26 N_b\).
- \(L_{\text{hit}}(\text{level})\) – access latency from different memory hierarchy levels (L1, L2, L3, local DRAM, remote DRAM). Typical values: 4 cycles (L1), 12 cycles (L2), 30–40 cycles (L3), 60+ cycles (local DRAM).
- \(W\) – hardware prefetch width (number of cache lines that can be brought in concurrently).
- \(C_{\text{ind}}\) – cost of an indirect access (e.g., an extra load from a translation table). Measured in cycles and depends on how well the table fits in cache.
- \(t_{\text{step}}\) – time of one simulation step (excluding reorganisation overhead). Ideally, the overhead of structure maintenance (split/merge, reorganisation) should not exceed, say, 5% of \(t_{\text{step}}\).

### Placement Quality Metrics

To quantify how close the physical layout of blocks is to the ideal (neighbors in the tree are neighbors in memory), we introduce:

- **Average distance between neighbors in address space**:
  \[
  D_{\text{avg}} = \frac{1}{M} \sum_{(l,n)} |\text{index}(l) - \text{index}(n)|
  \]
  where \(\text{index}(l)\) is the block index in the pool (or virtual address divided by block size). The smaller \(D_{\text{avg}}\), the higher the spatial locality. In the ideal case (all neighbors at a small constant offset) \(D_{\text{avg}}\) is of the order of a few units.

- **Fraction of neighbors that fall into the same cache bank / page**: useful for estimating potential conflicts.

- **Predictability coefficient** – for each neighbor pair we can evaluate whether the block address can be computed ahead of time without indirection (e.g., using a fixed offset). Let \(f_{\text{fast}}\) be the fraction of accesses that can use the fast path (constant offset). Then the total P2P time becomes:
  \[
  T_{\text{P2P}} = M \cdot \big( f_{\text{fast}} \cdot T_{\text{fast}} + (1-f_{\text{fast}}) \cdot T_{\text{slow}} \big).
  \]

### Memory Access Time Model Including Caching and Prefetch

For each neighbor block access, the access time can be approximated as:

\[
T_{\text{mem}}(l,n) =
\begin{cases}
L_{\text{hit}}(\text{cache}) & \text{if the block is already in cache (hit)} \\
L_{\text{miss}}(\text{level}) & \text{otherwise}
\end{cases}
\]

The hit probability depends on:

- the working set size (data of all leaves actively used in the current step),
- the prefetch strategy,
- the traversal order.

With fixed offsets and a linear traversal of neighbors, hardware prefetch can load subsequent blocks ahead of time, reducing effective latency. If accesses are random, prefetch is useless.

A simplified average access time can be estimated as:

\[
T_{\text{avg}} = \text{hit\_rate} \cdot L_{\text{hit}} + (1-\text{hit\_rate}) \cdot L_{\text{miss}}.
\]

With well‑organized access (e.g., Morton‑order traversal) the hit rate can be high due to spatial and temporal locality. With poor organisation it can be close to zero.

### Structure Maintenance Overhead

Let:

- \(R_{\text{split}}\) – average leaf split rate per step.
- \(c_{\text{alloc}}\) – cost of allocating a new block (cycles).
- \(c_{\text{update}}\) – cost of updating metadata (e.g., translation table, neighbor list).
- \(c_{\text{reorg}}\) – cost of a full pool reorganisation (moving blocks, updating all references).

Then the total overhead per step is:

\[
C_{\text{overhead}} = R_{\text{split}} \cdot (c_{\text{alloc}} + c_{\text{update}}) + \frac{c_{\text{reorg}}}{T_{\text{reorg}}}
\]

where \(T_{\text{reorg}}\) is the reorganisation period (in steps). This value should stay below a given threshold (e.g., 0.05·\(t_{\text{step}}\)) to avoid slowing down the simulation.

### Objective Function

Minimise the total step time including overhead:

\[
\min \; T_{\text{total}} = T_{\text{P2P}}(\text{layout}) + C_{\text{overhead}}(\text{policy})
\]

subject to constraints:

- Memory: pool size + metadata ≤ available RAM.
- Consistency: under parallel access no data races must occur (which imposes additional synchronisation costs).

### How the Model Can Be Used in Practice

1. **Collect profiling data**: measure current hit_rate, \(D_{\text{avg}}\), \(f_{\text{fast}}\) under different strategies (e.g., simple pool, two‑level indexing, periodic defragmentation).
2. **Calibrate parameters**: on test runs determine \(L_{\text{hit}}\) and \(L_{\text{miss}}\) for the target architecture.
3. **Estimate potential gain**: plug expected improvements from a new strategy (e.g., raising \(f_{\text{fast}}\) from 0.2 to 0.8) into the model to decide whether the effort is worthwhile.
4. **Dynamic adaptation**: based on current fragmentation (\(D_{\text{avg}}\)) decide when to trigger defragmentation, if the predicted gain in \(T_{\text{P2P}}\) outweighs the reorganisation cost.

### Why a Full Analytical Solution Is Impossible

Even with a refined model, challenges remain:

- Cache behavior depends on interleaved accesses from multiple threads and the scheduling scheme (e.g., TBB work stealing).
- The hit rate is a non‑linear function of access order and data volume.
- Split/merge frequency is hard to predict because it is driven by particle dynamics.

Nevertheless, the model provides a quantitative language for discussing trade‑offs and allows us to build heuristics that can be tuned to the specific problem.

We welcome feedback and ideas from the community, especially from those who have tackled similar problems in spatial data structures, memory allocators, or high‑performance graph traversal.

---

**Next steps:**  

- Prototype the two‑level indexing scheme and measure its overhead.  
- Investigate Morton‑based neighbor computation for different tree depths.  
- Experiment with periodic pool reorganisation to improve locality.
