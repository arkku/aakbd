#!/bin/sh
set -eu
cat <<EOF
// Auto-generated test runner -- do not edit
int main(int argc, char **argv) {
    for (int i = 1; i < argc; ++i)
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
            verbose = 1;
EOF
grep -E '^(static void )?test_[^ (]+\s*\(' "$1" | sed 's/.*test_\([^ (]*\).*/\1/' | while read name; do
    echo "    reset();"
    echo "    test_${name}();"
done
cat <<EOF
    (void) printf("\n%d of %d tests passed, %d failed\n",
        tests_run - tests_failed, tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
EOF
