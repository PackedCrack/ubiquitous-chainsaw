SET(STD_PCH <cstdint>
          <vector>
          <array>
          <chrono>
          <algorithm>
        <concepts>
        <type_traits>
        <stdexcept>
        <string>
        <string_view>
        <optional>
        <expected>
        <atomic>
        <cassert>
        <utility>
        <format>
        <memory>
        <unordered_map>
        <shared_mutex>
        <functional>
        <filesystem>
        <future>
        <variant>
        <cerrno>)

# --- functions --- #
function(use_pch PROJ)
  target_precompile_headers(${PROJ}
    PRIVATE
    ${STD_PCH}
  )
endfunction()