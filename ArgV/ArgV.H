#pragma once

void SplitIntoArgV(const char* cmdline, int& argc, char*** argv = nullptr, const char* **argstart = nullptr,
                   char* programName = nullptr)
