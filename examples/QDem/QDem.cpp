/* -*- coding: utf-8 -*-
 *
 * Copyright 2022 IBM RESEARCH. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTTypeSystemBuilder.h>

#include <cctype>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>

static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                       {"types", no_argument, 0, 'T'}};

static void PrintHelp() {
  std::cout << "Usage: QDem [-h |--help]";
  std::cout << " [-T |--types]";
  std::cout << " <mangled-string>" << std::endl;
}

static bool PrintTypes = false;
static std::string MS;

static unsigned ParseCommandLineArguments(int argc, char *const argv[]) {
  int C = 0;
  int option_index = 0;

  if (argc == 1) {
    PrintHelp();
    return 1;
  }

  while (1) {
    C = getopt_long(argc, argv, "hT", long_options, &option_index);
    if (C == -1)
      break;

    switch (C) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
      std::cerr << "Command-Line: Option " << long_options[option_index].name;
      if (optarg)
        std::cerr << " with Argument " << optarg;
      std::cerr << std::endl;
      break;
    case 'h':
      PrintHelp();
      return 2;
      break;
    case 'T':
      PrintTypes = true;
      break;
    case '?':
      std::cerr << "Command-Line Error: Invalid argument." << std::endl;
      return 1;
      break;
    default:
      return 1;
      break;
    }
  }

  if (optind < argc)
    MS = argv[optind++];

  if (MS.empty()) {
    std::cerr << "Command-Line Error: No mangled string specified."
              << std::endl;
    return 1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  using namespace QASM;
  unsigned R;

  if ((R = ParseCommandLineArguments(argc, argv)) != 0) {
    switch (R) {
    case 2:
      return 0;
      break;
    default:
      return 1;
      break;
    }
  }

  ASTObjectTracker::Instance().Enable();
  ASTTypeSystemBuilder::Instance().Init();

  ASTDemangler DM;
  DM.Demangle(MS);

  if (PrintTypes)
    DM.Print();

  std::cout << std::endl << DM.AsString() << std::endl << std::endl;

  ASTDemangledRegistry::Instance().Release();

  return 0;
}
