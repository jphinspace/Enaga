/**
 * @file   processor_editor_test_main.cpp
 * @brief  Entry point for processor/editor unit tests.
 */

#include <print>

bool RunProcessorCoreTests();
bool RunEditorActionsTests();

int main() {
  if (!RunProcessorCoreTests()) {
    std::println(stderr, "FAIL: processor core tests");
    return 1;
  }

  if (!RunEditorActionsTests()) {
    std::println(stderr, "FAIL: editor actions tests");
    return 1;
  }

  std::println("PASS: processor/editor unit tests");
  return 0;
}
