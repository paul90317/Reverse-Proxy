#!/bin/bash
# pre-commit: astyle + cppcheck (entire project)

# 設定要處理的目錄或檔案
TARGET_DIRS="src include"

# cppcheck 函數
run_cppcheck() {
    local target_file=$1
    echo "Checking $target_file..."
    
    cppcheck \
        --enable=all \
        --inconclusive \
        --std=c++17 \
        --library=std.cfg \
        --library=posix.cfg \
        -I include \
        --suppress=missingIncludeSystem \
        --quiet \
        --error-exitcode=1 \
        "$target_file" include/*.hpp
}

# 1. 自動格式化整個專案
echo "Running astyle on entire project..."
astyle --options=.astylerc $(find $TARGET_DIRS -type f -regex '.*\.\(c\|cpp\|h\|hpp\)$')

# 2. 靜態程式碼檢查
echo "Running cppcheck on entire project..."

# 檢查所有目標檔案
run_cppcheck "src/echo_server.cpp"
run_cppcheck "src/proxy_server.cpp"  
run_cppcheck "src/expose.cpp"

CPPCHECK_RESULT=$?

if [ $CPPCHECK_RESULT -ne 0 ]; then
    echo "cppcheck found issues. Please fix them before commit."
    exit 1
fi

echo "Pre-commit checks passed!"
exit 0