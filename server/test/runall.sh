cd ./bin || { echo "Directory './bin' not found."; exit 1; }

# Find all .test files and iterate over them
for test_file in *.test; do
    echo ""
    echo "-------------------------------------------------"
    echo "Running test file: $test_file"
    ./"$test_file"
done
