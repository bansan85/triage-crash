/* Copyright 2018 LE GARREC Vincent
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// C++ system
#include <cstring>
#include <iostream>

// lib2lgcgdb
#include <set_stack.h>

int main(int argc, char *argv[]) {
  std::string filename, folder;
  bool with_source_only = false;
  size_t top_frame = std::numeric_limits<size_t>::max();
  size_t bottom_frame = std::numeric_limits<size_t>::max();

  for (int i = 1; i < argc; i++) {
    // Filename
    if (strcmp(argv[i], "--source-only") == 0) {
      with_source_only = true;
    } else if (strncmp(argv[i], "--top-frame=", sizeof("--top-frame=") - 1) ==
               0) {
      try {
        top_frame = static_cast<size_t>(
            std::stoi(&argv[i][sizeof("--top-frame=") - 1], nullptr, 10));
      } catch (const std::invalid_argument &) {
        std::cerr << "Invalid argument for --top-frame" << std::endl;
        return 1;
      }
    } else if (strncmp(argv[i],
                       "--bottom-frame=", sizeof("--bottom-frame=") - 1) == 0) {
      try {
        bottom_frame = static_cast<size_t>(
            std::stoi(&argv[i][sizeof("--bottom-frame=") - 1], nullptr, 10));
      } catch (const std::invalid_argument &) {
        std::cerr << "Invalid argument for --bottom-frame" << std::endl;
        return 1;
      }
    } else if (strncmp(argv[i], "--folder=", sizeof("--folder=") - 1) == 0) {
      folder.assign(&argv[i][sizeof("--folder=") - 1]);
    } else if (strcmp(argv[i], "--help") == 0) {
      std::cout
          << "Usage: triage-crash [option] folder\n"
             "\n"
             "  --source-only Ignore frames that don't have known source "
             "file.\n"
             "\n"
             "  --top-frame=number Maximum number of frames to compare with,\n"
             "                     starting from the top. 2^32 by default.\n"
             "\n"
             "  --bottom-frame=number Maximum number of frames to compare "
             "with,\n"
             "                        starting from the bottom. 2^32 by "
             "default.\n"
             "\n"
             "  --folder=directory Folder that contains all *.btfull file.";
      return 1;
    } else {
      filename.assign(argv[i]);
    }
  }

  SetStack set_stack(with_source_only, top_frame, bottom_frame);

  if (folder.length() != 0) {
    set_stack.AddRecursive(folder);
  } else {
    set_stack.Add(filename);
  }

  set_stack.Print();

  return 0;
}
