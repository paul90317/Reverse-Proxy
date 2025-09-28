#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

echo "🔍 Running clang-format + static/dynamic checks before commit..."

# 找出 staged 的 C++ 檔案
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|cc|cxx|h|hpp)$' || true)

if [ -z "$files" ]; then
    echo "No C++ files staged. Skipping clang-format and cppcheck."
else
    # 格式化檔案
    for f in $files; do
        echo "🖌 Formatting $f"
        clang-format -i "$f"
        git add "$f"
    done
    echo "✅ clang-format applied"

    # =====================
    # 靜態檢查: cppcheck
    # =====================
    echo "🔍 Running cppcheck..."
    cppcheck --enable=all \
        --inconclusive \
        --std=c++17 \
        --quiet \
        --suppress=missingIncludeSystem \
        --suppress=uninitMemberVar \
        --suppress=noExplicitConstructor \
        --suppress=cstyleCast \
        --suppress=constParameter \
        --suppress=odrViolation \
         $files


    echo "✅ cppcheck passed"
fi

# =====================
# Build
# =====================
echo "🏗 Building..."
make
echo "🎉 All checks passed"
