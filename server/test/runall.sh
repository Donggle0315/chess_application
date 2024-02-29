cd ./bin || { echo "Directory './bin' not found."; exit 1; }


failed=0
# Find all .test files and iterate over them
for test_file in *.test; do
    echo ""
    echo "-------------------------------------------------"
    echo "Running test file: $test_file"
    ./"$test_file"
    if [ $? -ne 0 ]; then
        failed=1
    fi
done


if [ "$failed" -eq 1 ]; then
    echo "There was a failed test!"
else
    echo "All Tests Passed!"
fi   
