SET(STD_PCH <cstdint>
          <vector>
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
        <cerrno>)

# --- functions --- #
function(use_pch PROJ)
  target_precompile_headers(${PROJ}
    PRIVATE
    ${STD_PCH}
  )
endfunction()