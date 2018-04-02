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

#include <2lgc/pattern/publisher/connector_direct.h>
#include <2lgc/pattern/publisher/publisher_base.h>    // IWYU pragma: keep
#include <2lgc/pattern/publisher/publisher_remote.h>  // IWYU pragma: keep
#include <2lgc/pattern/publisher/subscriber_direct.h>
#include <2lgc/poco/gdb.pb.h>
#include <2lgc/software/gdb/gdb.h>
#include <2lgc/software/gdb/set_stack.h>
#include <assert.h>
#include <2lgc/pattern/publisher/connector_direct.cc>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

template class llgc::pattern::publisher::ConnectorDirect<msg::software::Gdbs>;

enum class Action
{
  NONE,
  GDB,
  SORT
};

class SubscriberBase final : public llgc::pattern::publisher::SubscriberDirect
{
 public:
  explicit SubscriberBase(uint32_t id) : SubscriberDirect(id) {}

  void Listen(const std::shared_ptr<const std::string> &message) override
  {
    msg::software::Gdbs messages_gdb;
    assert(messages_gdb.ParseFromString(*message.get()));
    for (int i = 0; i < messages_gdb.action_size(); i++)
    {
      auto gdbi = messages_gdb.action(i);
      switch (gdbi.data_case())
      {
        case msg::software::Gdb::kRunBtFull:
        {
          auto run_bt_fulli = gdbi.run_bt_full();
          for (int j = 0; j < run_bt_fulli.file_size(); j++)
          {
            std::cout << "Gdb timeout: " << run_bt_fulli.file(j) << std::endl;
          }
          break;
        }
        case msg::software::Gdb::kAddStack:
        {
          auto add_stacki = gdbi.add_stack();
          for (int j = 0; j < add_stacki.file_size(); j++)
          {
            std::cout << "Failed to read: " << add_stacki.file(j) << std::endl;
          }
          break;
        }
        case msg::software::Gdb::DATA_NOT_SET:
        default:
        {
          break;
        }
      }
    }
  }
};

int main(int argc, char *argv[])
{
  std::vector<std::string> filenames, folders, lists;
  bool with_source_only = false;
  bool parallel = false;
  bool print_one_by_group = false;
  int64_t timeout = 120;
  size_t top_frame = 10000;
  size_t bottom_frame = 0;
  Action action = Action::NONE;
  std::string regex;

  std::shared_ptr<SubscriberBase> subscriber =
      std::make_shared<SubscriberBase>(1);
  std::shared_ptr<
      llgc::pattern::publisher::ConnectorDirect<msg::software::Gdbs>>
      connector_gdb = std::make_shared<
          llgc::pattern::publisher::ConnectorDirect<msg::software::Gdbs>>(
          subscriber, llgc::software::gdb::Gdb::GetInstanceStatic());
  assert(connector_gdb->AddSubscriber(msg::software::Gdb::kRunBtFull));

  int i;
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "gdb") == 0)
    {
      if (action != Action::NONE)
      {
        std::cerr
            << "Only one action at a time (gdb or sort) can be specified.\n";
        return 1;
      }
      action = Action::GDB;
    }
    else if (strcmp(argv[i], "--") == 0)
    {
      if (action != Action::GDB)
      {
        std::cerr << "-- is only applicable with gdb action. See --help.\n";
        return 1;
      }
      break;
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
    else if (strncmp(argv[i], "--timeout=", sizeof("--timeout=") - 1) == 0)
    {
      if (action != Action::GDB)
      {
        std::cerr
            << "--timeout is only applicable with gdb action. See --help.\n";
        return 1;
      }
      timeout = std::stoll(&argv[i][sizeof("--timeout=") - 1]);
    }
    else if (strcmp(argv[i], "--help") == 0)
    {
      std::cout
          << "Usage:  triage-crash --help\n"
             "        triage-crash gdb [options] <--file=file folder=dir> -- "
             "/path/app @@\n"
             "        @@ will be replaced by the file.\n"
             "        triage-crash sort [options] <--file=file folder=dir>\n"
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
             "               If empty (by default), all files are read.\n"
             "               Must be compliant with ECMAScript (javascript).\n"
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
             "Options for gdb:\n"
             "  --timeout=time  Timeout in second for execution of each gdb "
             "instance.\n"
             "                  By default: 120s. Max: 2^61s.\n"
             "\n"
             "Options for sort:\n"
             "  --source-only          Ignore frames that don't have known "
             "source file.\n"
             "                         By default: not ignore.\n"
             "  --top-frame=number     Maximum number of frames to compare "
             "with,\n"
             "                         starting from the top. 10000 by "
             "default.\n"
             "  --bottom-frame=number  Maximum number of frames to compare "
             "with,\n"
             "                         starting from the bottom. 0 by "
             "default.\n"
             "  --print-one-by-group   Show only one btfull file by group.\n"
             "                         Reduce memory usage.\n"
             "\n"
             "Files:\n"
             "  --folder=directory  Folder that contains all *.btfull file.\n"
             "                      Add as many --folder you want.\n"
             "  --file=file         Add a single file. Add as many --file you "
             "want.\n"
             "  --list=file         Add many files from a text file. --regex is"
             " not used.\n";
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
    else if (strncmp(argv[i], "--list=", sizeof("--list=") - 1) == 0)
    {
      if (action == Action::NONE)
      {
        std::cerr << "--list is only applicable with an action. See --help.\n";
        return 1;
      }
      lists.push_back(&argv[i][sizeof("--list=") - 1]);
    }
    else
    {
      std::cerr << "Invalid argument: " << argv[i]
                << ".\n"
                   "Run triage-crash with --help.\n";
      return 1;
    }
  }

  const unsigned int nthreads =
      parallel ? std::numeric_limits<unsigned int>::max() : 1;

  if (action == Action::SORT)
  {
    int retval = 0;
    llgc::software::gdb::SetStack set_stack(with_source_only, top_frame,
                                            bottom_frame);

    std::shared_ptr<
        llgc::pattern::publisher::ConnectorDirect<msg::software::Gdbs>>
        connector_stack = std::make_shared<
            llgc::pattern::publisher::ConnectorDirect<msg::software::Gdbs>>(
            subscriber, set_stack.GetInstanceLocal());
    assert(connector_stack->AddSubscriber(msg::software::Gdb::kAddStack));

    for (const std::string &folder : folders)
    {
      if (!set_stack.AddRecursive(folder, nthreads, regex, print_one_by_group))
      {
        std::cerr << "Failed to read some files in folder " << folder << "."
                  << std::endl;
        retval = 1;
      }
    }
    for (const std::string &filename : filenames)
    {
      if (!set_stack.Add(filename, print_one_by_group))
      {
        std::cerr << "Failed to read file " << filename << "." << std::endl;
        retval = 1;
      }
    }
    for (const std::string &list : lists)
    {
      if (!set_stack.AddList(list, nthreads, print_one_by_group))
      {
        std::cerr << "Failed to read some files in list " << list << "."
                  << std::endl;
        retval = 1;
      }
    }

    set_stack.Print();

    return retval;
  }
  else if (action == Action::GDB)
  {
    if (i == argc)
    {
      std::cerr << "Missing -- to know what application to run under gdb."
                << std::endl;
      return 1;
    }
    else if (i == argc - 1)
    {
      std::cerr << "Missing application to run under gdb after -- ."
                << std::endl;
      return 1;
    }

    int j;
    for (j = i + 1; j < argc; j++)
    {
      if (strcmp(argv[j], "@@") == 0)
      {
        break;
      }
    }
    if (j == argc)
    {
      std::cerr << "Missing @@ in the arguments." << std::endl;
      return 1;
    }

    int retval = 0;

    for (const std::string &folder : folders)
    {
      if (!llgc::software::gdb::Gdb::RunBtFullRecursive(
              folder, nthreads, regex, static_cast<unsigned int>(argc - i - 1),
              &argv[i + 1], timeout))
      {
        std::cerr << "Failed to run gdb with some files in folder " << folder
                  << "." << std::endl;
        retval = 1;
      }
    }
    for (const std::string &filename : filenames)
    {
      if (!llgc::software::gdb::Gdb::RunBtFull(
              filename, static_cast<unsigned int>(argc - i - 1), &argv[i + 1],
              timeout))
      {
        std::cerr << "Failed to read file " << filename << "." << std::endl;
        retval = 1;
      }
    }
    for (const std::string &list : lists)
    {
      if (!llgc::software::gdb::Gdb::RunBtFullList(
              list, nthreads, static_cast<unsigned int>(argc - i - 1),
              &argv[i + 1], timeout))
      {
        std::cerr << "Failed to run gdb with some files in folder " << list
                  << "." << std::endl;
        retval = 1;
      }
    }

    return retval;
  }

  return 0;
}
