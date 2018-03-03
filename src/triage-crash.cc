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

// lib2lgcgdb
#include <set_stack.h>

#include <experimental/filesystem>

// C++ system
#include <cstring>
#include <iostream>

// gdb -batch-silent -ex "run" -ex "set logging overwrite on" -ex "set logging
// file $2.btfull" -ex "set logging on" -ex "set pagination off" -ex "handle
// SIG33 pass nostop noprint" -ex "backtrace full" -ex "set logging off" -ex
// "quit" --args $@

enum class Action
{
  NONE,
  GDB,
  SORT
};

int main(int argc, char *argv[])
{
  std::vector<std::string> filenames, folders;
  bool with_source_only = false;
  bool parallel = false;
  bool print_one_by_group = false;
  size_t top_frame = std::numeric_limits<size_t>::max();
  size_t bottom_frame = std::numeric_limits<size_t>::max();
  Action action = Action::NONE;
  std::string regex;

  for (int i = 1; i < argc; i++)
  {
    // Filename
    if (strcmp(argv[i], "gdb") == 0)
    {
      if (action != Action::NONE)
      {
        std::cerr
            << "Only one action at a time (gdb or sort) must be specified.\n";
        return 1;
      }
      action = Action::GDB;
    }
    else if (strcmp(argv[i], "sort") == 0)
    {
      if (action != Action::NONE)
      {
        std::cerr
            << "Only one action at a time (gdb or sort) must be specified.\n";
        return 1;
      }
      action = Action::SORT;
    }
    else if (strcmp(argv[i], "--source-only") == 0)
    {
      if (action != Action::SORT)
      {
        std::cerr << "--source-only is only applicable with sort action. See "
                     "--help.\n";
        return 1;
      }
      with_source_only = true;
    }
    else if (strcmp(argv[i], "--parallel") == 0)
    {
      parallel = true;
    }
    else if (strncmp(argv[i], "--regex=", sizeof("--regex=") - 1) == 0)
    {
      if (action == Action::NONE)
      {
        std::cerr
            << "--regex= is only applicable with an action. See --help.\n";
        return 1;
      }
      regex.assign(&argv[i][sizeof("--regex=") - 1]);
    }
    else if (strncmp(argv[i], "--top-frame=", sizeof("--top-frame=") - 1) == 0)
    {
      if (action != Action::SORT)
      {
        std::cerr
            << "--top-frame is only applicable with sort action. See --help.\n";
        return 1;
      }
      try
      {
        top_frame = static_cast<size_t>(
            std::stoi(&argv[i][sizeof("--top-frame=") - 1], nullptr, 10));
      }
      catch (const std::invalid_argument &)
      {
        std::cerr << "Invalid argument for --top-frame" << std::endl;
        return 1;
      }
    }
    else if (strncmp(argv[i],
                     "--bottom-frame=", sizeof("--bottom-frame=") - 1) == 0)
    {
      if (action != Action::SORT)
      {
        std::cerr << "--bottom-frame is only applicable with sort action. See "
                     "--help.\n";
        return 1;
      }
      try
      {
        bottom_frame = static_cast<size_t>(
            std::stoi(&argv[i][sizeof("--bottom-frame=") - 1], nullptr, 10));
      }
      catch (const std::invalid_argument &)
      {
        std::cerr << "Invalid argument for --bottom-frame" << std::endl;
        return 1;
      }
    }
    else if (strcmp(argv[i], "--print-one-by-group") == 0)
    {
      if (action != Action::SORT)
      {
        std::cerr << "--print-one-by-group is only applicable with sort "
                     "action. See --help.\n";
        return 1;
      }
      print_one_by_group = true;
    }
    else if (strncmp(argv[i], "--folder=", sizeof("--folder=") - 1) == 0)
    {
      if (action == Action::NONE)
      {
        std::cerr
            << "--folder is only applicable with an action. See --help.\n";
        return 1;
      }
      folders.push_back(&argv[i][sizeof("--folder=") - 1]);
    }
    else if (strcmp(argv[i], "--help") == 0)
    {
      std::cout
          << "Usage:  triage-crash --help\n"
             "        triage-crash gdb [options] <--file=file folder=folder>\n"
             "        triage-crash sort [options] <--file=file folder=folder>\n"
             "\n"
             "  --help  This help.\n"
             "\n"
             "Action:\n"
             "  gdb   Run gdb with files to extract full backtrace.\n"
             "  sort  Sort btfull files and show result by group.\n"
             "\n"
             "Common options:\n"
             "  --parallel   Read folder with the number of threads that allow "
             "the CPU.\n"
             "               Default false.\n"
             "  --regex=str  With --folder only. Read only files that match "
             "str.\n"
             "               str is used in the C++ function std::regex_match "
             "without any\n"
             "               changes.\n"
             "               * With gdb action and afl's crashes, you can use\n"
             "               --regex=^id(?!.*btfull).*$ (all files that starts "
             "with id and\n"
             "               not ends with .btfull). From bash, use:\n"
             "               --regex=^id\\(?\\!.*btfull\\).*$\n"
             "               * With sort action and afl's crashes, you "
             "can use\n"
             "               --regex=^id.*\\.btfull$ (all files that starts "
             "with id and\n"
             "               ends with .btfull)\n"
             "\n"
             "Options for sort:\n"
             "  --source-only          Ignore frames that don't have known "
             "source file.\n"
             "                         By default: not ignore.\n"
             "  --top-frame=number     Maximum number of frames to compare "
             "with,\n"
             "                         starting from the top. 2^32 by "
             "default.\n"
             "  --bottom-frame=number  Maximum number of frames to compare "
             "with,\n"
             "                         starting from the bottom. 2^32 by "
             "default.\n"
             "  --print-one-by-group   Show only one btfull file by group.\n"
             "\n"
             "Files:\n"
             "  --folder=directory  Folder that contains all *.btfull file.\n"
             "                      Add as many --folder you want.\n"
             "  --file=file         Add a single file. Add as many --file you "
             "want.\n"
             "\n";
      return 1;
    }
    else if (strncmp(argv[i], "--file=", sizeof("--file=") - 1) == 0)
    {
      if (action == Action::NONE)
      {
        std::cerr << "--file is only applicable with an action. See --help.\n";
        return 1;
      }
      filenames.push_back(&argv[i][sizeof("--file=") - 1]);
    }
    else
    {
      std::cerr << "Invalid argument: " << argv[i]
                << ".\n"
                   "Run triage-crash with --help.\n";
      return 1;
    }
  }

  if (action == Action::SORT)
  {
    SetStack set_stack(with_source_only, top_frame, bottom_frame);

    const unsigned int nthreads =
        parallel ? std::numeric_limits<unsigned int>::max() : 1;

    for (const std::string &folder : folders)
    {
      if (!set_stack.AddRecursive(folder, nthreads, regex))
      {
        std::cerr << "Failed to read some files in " << folder << "."
                  << std::endl;
        return 1;
      }
    }
    for (const std::string &filename : filenames)
    {
      if (!set_stack.Add(filename))
      {
        std::cerr << "Failed to read " << filename << "." << std::endl;
        return 1;
      }
    }

    set_stack.Print(print_one_by_group);
  }
  else if (action == Action::GDB)
  {
  }

  return 0;
}
