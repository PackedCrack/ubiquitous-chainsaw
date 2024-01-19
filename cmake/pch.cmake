SET(STD_PCH <cstdint>
          <vector>
          <random>
          <chrono>
          <array>
          <set>
          <unordered_map>
          <algorithm>
          <type_traits>
        <concepts>
        <stdexcept>
        <string>
        <string_view>
        <numeric>
        <optional>
        <cassert>)

# --- functions --- #
function(use_pch PROJ)
  target_precompile_headers(${PROJ}
    PRIVATE
    ${STD_PCH}
  )
endfunction()