SET(STD_PCH <cstdint>
          <vector>
          <array>
          <chrono>
          <algorithm>
        <concepts>
        <stdexcept>
        <string>
        <string_view>
        <expected>
        <atomic>
        <cassert>
        <utility>
        <format>
        <memory>
        <unordered_map>
        <shared_mutex>
        <functional>
        <cerrno>)

# --- functions --- #
function(use_pch PROJ)
  target_precompile_headers(${PROJ}
    PRIVATE
    ${STD_PCH}
  )
endfunction()