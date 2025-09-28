#!/bin/bash
set -e

echo "🔍 Running clang-format + static/dynamic checks before commit..."

# 找出 staged 的 C++ 檔案
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|cc|cxx|h|hpp)$' || true)

if [ -z "$files" ]; then
    echo "No C++ files staged. Skipping clang-format and cppcheck."
else
    # 格式化檔案
    for f in $files; do
        clang-format -i "$f"
        git add "$f"
    done
    echo "✅ clang-format applied"

    # =====================
    # 靜態檢查: cppcheck
    # =====================
    echo "🔍 Running cppcheck..."
    cppcheck --enable=all --inconclusive --std=c++17 --quiet $files || {
        echo "❌ cppcheck failed"
        exit 1
    }
    echo "✅ cppcheck passed"
fi

echo "building..."

make

echo "🎉 All checks passed"
