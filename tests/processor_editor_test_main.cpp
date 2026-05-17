/**
 * @file   processor_editor_test_main.cpp
 * @brief  Entry point for processor/editor unit tests.
 */

#include <cstdio>

bool RunProcessorCoreTests();
bool RunEditorActionsTests();

int main() {
  if (!RunProcessorCoreTests()) {
    std::fprintf(stderr, "FAIL: processor core tests\n");
    return 1;
  }

  if (!RunEditorActionsTests()) {
    std::fprintf(stderr, "FAIL: editor actions tests\n");
    return 1;
  }

  std::puts("PASS: processor/editor unit tests");
  return 0;
}
