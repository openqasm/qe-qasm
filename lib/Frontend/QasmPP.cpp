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

#include <qasm/QPP/QasmPP.h>
#include <qasm/QPP/QasmPPFileCleaner.h>
#include <qasm/AST/ASTOpenQASMVersionTracker.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <filesystem>
#include <memory>
#include <random>
#include <regex>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cassert>

namespace QASM {

QasmPreprocessor QasmPreprocessor::QPP;
std::string QasmPreprocessor::CurrentFilePath("");

static std::vector<std::unique_ptr<std::fstream>> Tmpfiles;
static std::vector<std::string> TmpFilenames;
static std::map<std::string, uint32_t> LineMap;
static bool CCS = false;
static bool HOP = false;
static bool HCL = false;

static const std::regex CREGX(u8"creg[ ]+[a-zA-Z_]+\\[[0-9]+\\]");
static const std::regex QREGX(u8"qreg[ ]+[a-zA-Z_]+\\[[0-9]+\\]");
static const std::regex CBITX(u8"cbit[ ]+[a-zA-Z_]+\\[[0-9]+\\]");
static const std::regex BITX(u8"bit[ ]+[a-zA-Z_]+\\[[0-9]+\\]");
static const std::regex QELX(u8"qelib1\\.inc");

static const std::regex INCX0("^(\\s*)include[ ]+\"(\\S*)\"(\\s*);$");
static const std::regex INCX1("^(\\s*)include[ ]+\"(\\S*)\"(\\s*);(\\s*)$");
static const std::regex INCX11("^(\\s*)include[ ]+\"(\\S*)\";(\\s*)//");
static const std::regex INCX12("^(\\s*)include[ ]+\"(\\S*)\";(\\s*)//(.*)");
static const std::regex INCX13("^(\\s*)include[ ]+\"(\\S*)\";(\\s*)(/\\*)(\\s*)");
static const std::regex INCX17("^(\\s*)include[ ]+\"(\\S*)\";(\\s*)(/\\*)(.*)");

static const std::regex CCSOPX0("/\\*");
static const std::regex CCSOPX1("/\\*(.*)");
static const std::regex CCSCLX("/\\*(.*)\\*/");

QasmPreprocessor::QasmPreprocessor()
  : IncludePaths(), QPR() {
    srand48(std::time(0));
}

static void Sanitize(std::string& Str) {
  if (Str.empty())
    return;

  std::string Ret;

  for (std::string::size_type I = 0; I < Str.size(); ++I) {
    if (Str[I] != '\"' && Str[I] != ' ' && Str[I] != ';' && Str[I] != '\'')
      Ret.insert(Ret.end(), Str[I]);
  }

  Str = Ret;
}

static bool ValidateRegexIncludeLine(const std::string& Line,
                                     std::string& Filename,
                                     std::string& Comment) {
  std::smatch INCMX;

  if (std::regex_match(Line, INCMX, INCX0)) {
    // Plain include directive with no additional spaces
    // and no comments.
    Filename = INCMX[2].str();
    assert(!Filename.empty() && "Invalid regex subexpression match!");
    return !Filename.empty();
  }

  if (std::regex_match(Line, INCMX, INCX1)) {
    // Plain include directive with additional spaces
    // and no comments.
    Filename = INCMX[2].str();
    assert(!Filename.empty() && "Invalid regex subexpression match!");
    return !Filename.empty();
  }

  if (std::regex_match(Line, INCMX, INCX11)) {
    // Include directive with C++ comment with no characters.
    Filename = INCMX[2].str();
    assert(!Filename.empty() && "Invalid regex subexpression match!");
    return !Filename.empty();
  }

  if (std::regex_match(Line, INCMX, INCX12)) {
    // Include directive with C++ comment with characters.
    Filename = INCMX[2].str();
    assert(!Filename.empty() && "Invalid regex subexpression match!");
    return !Filename.empty();
  }

  if (std::regex_match(Line, INCMX, INCX13)) {
    // Include directive with an open C comment.
    HOP = true;
    Filename = INCMX[2].str();
    Comment = INCMX[4].str();
    assert(!Filename.empty() && "Invalid regex subexpression match!");
    assert(!Comment.empty() && "Invalid regex subexpression match!");
    return !Filename.empty();
  }

  if (std::regex_search(Line, INCMX, INCX17)) {
    // Include directive with either an open or closed C comment.
    Filename = INCMX[2].str();
    assert(!Filename.empty() && "Invalid regex subexpression match!");
    std::smatch CCMX;

    if (std::regex_search(Line, CCMX, CCSCLX)) {
      HCL = true;
      Comment = CCMX[0].str();
      assert(!Comment.empty() && "Invalid regex subexpression match!");
      return !Filename.empty();
    }

    if (std::regex_search(Line, CCMX, CCSOPX1)) {
      HOP = true;
      Comment = CCMX[0].str();
      assert(!Comment.empty() && "Invalid regex subexpression match!");
      return !Filename.empty();
    }

    if (std::regex_search(Line, CCMX, CCSOPX0)) {
      HOP = true;
      Comment = "/*\n";
      return !Filename.empty();
    }
  }

  return false;
}

static bool ValidateIncludeLine(const std::string& Line,
                                std::string& Filename,
                                std::string& Comment) {
  Filename.clear();
  Comment.clear();

  if (!ValidateRegexIncludeLine(Line, Filename, Comment)) {
    std::cerr << "Error: [QasmPP]: Malformed `include` directive."
      << std::endl;
    return false;
  }

  size_t QPB = Line.find_first_of("\"");
  if (QPB == std::string::npos) {
    std::cerr << "Error: [QasmPP]: Malformed `include` directive."
      << std::endl;
    return false;
  }

  size_t QPE = Line.find_last_of("\"");
  if (QPE == std::string::npos || QPE <= QPB || QPE - QPB <= 1) {
    std::cerr << "Error: [QasmPP]: Malformed `include` directive."
      << std::endl;
    return false;
  }

  return true;
}

static bool IsComment(const std::string& Line) {
  if (Line.length() > 1U) {
    if (Line.find_first_not_of(u8' ') == std::string::npos)
      return CCS;

    if (Line[Line.find_first_not_of(u8' ')] == u8'/') {
      if (Line.length() >= 2U &&
          Line[Line.find_first_not_of(u8' ') + 1] == u8'/') {
        return true;
      }

      return false;
    } else if (Line[Line.find_first_not_of(u8' ')] == u8'/' &&
             Line[Line.find_first_not_of(u8' ') + 1] == u8'*') {
      CCS = true;
      return true;
    } else if (Line[Line.find_first_not_of(u8' ')] == u8'*' && CCS) {
      return true;
    } else if (Line[Line.find_first_not_of(u8' ')] == u8'*' &&
               Line[Line.find_first_not_of(u8' ') + 1] == u8'/' && CCS) {
      CCS = false;
      return true;
    } else if ((Line.find(u8"*/") != std::string::npos) && CCS) {
      CCS = false;
      return true;
    } else if (CCS) {
      return true;
    }

    CCS = false;
    return false;
  }

  CCS = false;
  return false;
}

void CleanupTemporaryFiles() {
  for (std::unique_ptr<std::fstream>& FP : Tmpfiles)
    FP->close();

  Tmpfiles.clear();

  for (const std::string& FS : TmpFilenames)
    std::remove(FS.c_str());

  TmpFilenames.clear();
}

static bool CheckFile(std::fstream& File) {
  if (!File.good()) {
    std::cerr << "Error: [QasmPP]: Bad File!" << std::endl;
    return false;
  }

  if (File.eof()) {
    std::cerr << "Error: [QasmPP]: File Already at EOF!" << std::endl;
    return false;
  }

  return true;
}

static bool CheckFile(std::ifstream& File) {
  if (!File.good()) {
    std::cerr << "Error: [QasmPP]: Bad File!" << std::endl;
    return false;
  }

  if (File.eof()) {
    std::cerr << "Error: [QasmPP]: File Already at EOF!" << std::endl;
    return false;
  }

  return true;
}

static double DetermineOpenQASMVersion(const char* B) {
  std::string OQS = B ? B : "";

  if (OQS.empty())
    return 3.0;

  std::smatch CSMX;
  std::smatch QSMX;
  std::smatch CBMX;
  std::smatch BMX;
  std::smatch QELMX;

  if (std::regex_search(OQS, CSMX, CREGX)) {
    return 2.0;
  } else if (std::regex_search(OQS, QSMX, QREGX)) {
    return 2.0;
  } else if (std::regex_search(OQS, CBMX, CBITX)) {
    return 2.0;
  } else if (std::regex_search(OQS, BMX, BITX)) {
    return 2.0;
  } else if (std::regex_search(OQS, QELMX, QELX)) {
    return 2.0;
  }

  return 3.0;
}

std::string GenRandomString(unsigned Len) {
  static const char AlNum[] = u8"0123456789"
                              u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              u8"abcdefghijklmnopqrstuvwxyz"
                              u8"-_+.";

  static thread_local std::default_random_engine
    RandomEngine(std::random_device{}());
  static thread_local std::uniform_int_distribution<int>
    RandomDistribution(0, sizeof(AlNum) - 2);

  std::string Tmp(Len < 19 ? 19 : Len, '\0');

  for (std::string::value_type& C : Tmp)
    C = AlNum[RandomDistribution(RandomEngine)];

  return Tmp;
}

std::string QasmPreprocessor::GenTempFilename(unsigned Length,
                                              const char* Suffix) {
  const char* Tmpdir = ::getenv("TMPDIR");
  if (!Tmpdir)
    Tmpdir = "/tmp";

  std::string Tmp = Tmpdir;
  Tmp += "/";
  Tmp += GenRandomString(Length);

  if (Suffix && *Suffix)
    Tmp += Suffix;

  return Tmp;
}

void QasmPreprocessor::SetFilePath(std::string& FilePath) {
  FilePath = QasmPreprocessor::CurrentFilePath;
}

bool QasmPreprocessor::Preprocess(std::istream* InStream) {
  if (!InStream)
    return false;

  std::string StreamFilename = GenTempFilename(19);
  std::fstream::openmode Openmode =
    std::fstream::in | std::fstream::out | std::fstream::trunc;
  std::fstream FS(StreamFilename.c_str(), Openmode);

  if (!CheckFile(FS)) {
    CleanupTemporaryFiles();
    return false;
  }

  TmpFilenames.push_back(StreamFilename);
  FS << InStream->rdbuf() << "\n";
  FS.flush();
  FS.close();

  Openmode = std::ifstream::in;
  std::ifstream* IFS = new std::ifstream(StreamFilename.c_str(), Openmode);
  assert(IFS && "Could not create a valid std::ifstream Input Stream!");

  if (!CheckFile(*IFS)) {
    CleanupTemporaryFiles();
    return false;
  }

  IFS->rdbuf()->pubseekpos(0, IFS->in);
  bool R = Preprocess(*IFS);
  SetTranslationUnit(IFS);
  return R;
}

bool QasmPreprocessor::Preprocess(std::ifstream& InFile) {
  if (!CheckFile(InFile))
    return false;

  unsigned IC;
  uint32_t LC = 0U;
  unsigned Pass = 0;
  bool OQF = false;
  int32_t OQV = 3;

  std::set<std::string> AlreadySeen;
  std::string MainTU = DIAGLineCounter::Instance().GetTranslationUnit();
  std::string PPString;
  std::string Tmpname;
  std::fstream::openmode Openmode =
    std::fstream::in | std::fstream::out | std::fstream::trunc;

  std::string WorkFilename = GenTempFilename(19);
  std::fstream WorkFile(WorkFilename.c_str(), Openmode);

  if (!CheckFile(WorkFile)) {
    CleanupTemporaryFiles();
    return false;
  }

  TmpFilenames.push_back(WorkFilename);

  std::filebuf* PBuf = InFile.rdbuf();
  std::size_t Size = PBuf->pubseekoff(0, InFile.end, InFile.in);
  PBuf->pubseekpos(0, InFile.in);
  char* Buffer = new char[Size + 8];
  (void) memset(Buffer, 0, Size + 8);
  PBuf->sgetn(Buffer, Size - 1);

  WorkFile << Buffer << "\n";
  WorkFile.flush();
  PBuf = WorkFile.rdbuf();
  PBuf->pubseekpos(0, WorkFile.in);

  OQV = static_cast<int32_t>(DetermineOpenQASMVersion(Buffer));
  delete[] Buffer;

  do {
    IC = 0;
    Tmpname = GenTempFilename(19);
    std::string Line;

    Tmpfiles.push_back(std::make_unique<std::fstream>());
    std::fstream& Tmpfile = *Tmpfiles[Pass++].get();
    Tmpfile.open(Tmpname.c_str(), Openmode);

    if (!CheckFile(Tmpfile)) {
      CleanupTemporaryFiles();
      return false;
    }

    TmpFilenames.push_back(Tmpname);
    std::ifstream IncludedFile;

    LineMap[MainTU] = LC;

    while (std::getline(WorkFile, Line)) {
      if (IsComment(Line)) {
        LineMap[MainTU] = ++LC;
        Tmpfile << Line.c_str() << "\n";
        continue;
      }

      if (Line.empty() && !OQF) {
        LineMap[MainTU] = ++LC;
        continue;
      }

      if (!OQF) {
        if (Line.find("OPENQASM") == std::string::npos) {
          if (OQV == 3) {
            Tmpfile << "OPENQASM 3.0;\n\n";
            ASTOpenQASMVersionTracker::Instance().SetVersion(3.0);
          } else {
            Tmpfile << "OPENQASM 2.0;\n\n";
            ASTOpenQASMVersionTracker::Instance().SetVersion(2.0);
          }

          LineMap[MainTU] = LC;
          PPString = "\n#file: \"";
          PPString += MainTU;
          PPString += "\";\n";
          Tmpfile << PPString;

          PPString = "\n#line: ";
          PPString += std::to_string(LineMap[MainTU]);
          PPString += ";\n";
          Tmpfile << PPString;
          OQF = true;
        }
      }

      Tmpfile << Line.c_str() << "\n";
      LineMap[MainTU] = ++LC;

      if (Line.find("OPENQASM") != std::string::npos) {
        PPString = "\n#file: \"";
        PPString += MainTU;
        PPString += "\";\n";
        Tmpfile << PPString;

        PPString = "\n#line: ";
        PPString += std::to_string(LineMap[MainTU]);
        PPString += ";\n";
        Tmpfile << PPString;
        OQF = true;
        break;
      }
    }

    std::string Comment;
    while (std::getline(WorkFile, Line)) {
      if (HOP) {
        CCS = true;
        HOP = false;
      } else if (HCL) {
        HCL = false;
      }

      Comment.clear();

      if (IsComment(Line)) {
        LineMap[MainTU] = ++LC;
        Tmpfile << Line.c_str() << "\n";
        continue;
      }

      if (Line.find("include ") != std::string::npos) {
        ++IC;
        LineMap[MainTU] = ++LC;

        std::string File;
        if (!ValidateIncludeLine(Line, File, Comment)) {
          CleanupTemporaryFiles();
          WorkFile.close();
          return false;
        }

        Sanitize(File);

        std::string FoundPath = QPP.Resolver().ResolvePath(File);
        if (FoundPath.empty()) {
          std::cerr << "Error: [QasmPP]: " << File << ": File not found."
            << std::endl;
          CleanupTemporaryFiles();
          return false;
        }

        if (AlreadySeen.find(FoundPath) != AlreadySeen.end())
          continue;

        IncludedFile.open(FoundPath.c_str(), std::fstream::in);
        if (!CheckFile(IncludedFile)) {
          CleanupTemporaryFiles();
          return false;
        }

        LineMap[File] = 1;

        PPString = "\n\n#file: \"";
        PPString += File;
        PPString += "\";\n";
        Tmpfile << PPString;
        PPString = "\n#line: ";
        PPString += std::to_string(LineMap[File]);
        PPString += ";\n";
        Tmpfile << PPString;

        std::string IncLine;
        while (std::getline(IncludedFile, IncLine)) {
          Tmpfile << IncLine.c_str() << "\n";
        }

        IncludedFile.close();
        AlreadySeen.insert(FoundPath);
        Tmpfile << "\n";

        PPString = "\n#file: \"";
        PPString += MainTU;
        PPString += "\";\n";
        Tmpfile << PPString;

        unsigned LN = HOP ? LineMap[MainTU] - 1U : LineMap[MainTU];
        LineMap[MainTU] = ++LN;
        PPString = "\n#line: ";
        PPString += std::to_string(LineMap[MainTU]);
        PPString += ";\n";
        Tmpfile << PPString;

        if (HOP) {
          CCS = true;
          Tmpfile << Comment << "\n";
        }
      } else {
        Tmpfile << Line.c_str() << "\n";
      }
    }

    if (IC > 0) {
      PBuf = Tmpfile.rdbuf();
      Size = PBuf->pubseekoff(0, Tmpfile.end, Tmpfile.in);
      PBuf->pubseekpos(0, Tmpfile.in);
      Buffer = new char[Size + 8];
      (void) memset(Buffer, 0, Size + 8);
      PBuf->sgetn(Buffer, Size - 1);

      WorkFile.flush();
      WorkFile.close();
      WorkFile.open(WorkFilename, Openmode);

      if (!CheckFile(WorkFile)) {
        CleanupTemporaryFiles();
        return false;
      }

      PBuf = WorkFile.rdbuf();
      PBuf->pubseekpos(0, WorkFile.in);
      WorkFile << Buffer << "\n";
      PBuf = WorkFile.rdbuf();
      PBuf->pubseekpos(0, WorkFile.in);
      delete[] Buffer;

      if (!CheckFile(WorkFile)) {
        CleanupTemporaryFiles();
        return false;
      }
    }
  } while (IC > 0);

  TmpFilenames.pop_back();

  std::fstream& Tmpfile = *Tmpfiles.back().get();
  PBuf = Tmpfile.rdbuf();
  Size = PBuf->pubseekoff(0, Tmpfile.end, Tmpfile.in);
  PBuf->pubseekpos(0, Tmpfile.in);

  Tmpfile.flush();
  Tmpfile.close();
  InFile.close();

  InFile.open(Tmpname.c_str(), std::fstream::in);

  if (!CheckFile(InFile)) {
    InFile.close();
    CleanupTemporaryFiles();
    return false;
  }

  QasmPPFileCleaner::Instance().RegisterFile(Tmpname);
  QasmPreprocessor::CurrentFilePath = Tmpname;

  PBuf = InFile.rdbuf();
  Size = PBuf->pubseekoff(0, InFile.end, InFile.in);
  PBuf->pubseekpos(0, InFile.in);
  CleanupTemporaryFiles();
  return true;
}

} // namespace QASM

