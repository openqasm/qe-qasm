#qss-qasm

Coding Style
============

Please follow the LLVM Coding Style Standard at:
  https://llvm.org/docs/CodingStandards.html#source-code-formatting

- Pull Requests must squash all commits into a single commit before
  submitting.
- Pull Requests must compile and link. If it doens't compile or link,
  it won't get approved.
- We do not follow all the LLVM restrictions on using C++ RTTI or
  C++ Exceptions, but we do follow their Code Formatting Standards.
- No commented out code in Pull Requests. If it's commented out, that
  means it doesn't belong in a Pull Request.
- Comments explaining what the code does are encouraged.
- No quotes around #include'd header files. Only '<' and '>'.
- Order of inclusion of header files:
  1. Local Project header files first.
  2. Imported Project header files after that.
  3. C++ Standard Library header files last.
- Header file guards (#ifndef HEADER_FILE_H etc) MUST avoid at all cost
  the possibility of collision with header file guards from other projects
  or libraries. As such, do not choose a simple, common preprocessor identifier
  for your header file guard. Follow LLVM's example and take a look at their
  header files. Using a Prefix specific to the project (for example:
  #ifndef \_\_QASM\_\<+OTHER-TOKENS-HERE\>) usually does the job.
- No tabs. Tabs mess-up the formatting because everyone sets their tab
  width differently.
- We use the CamelCase naming style for Function or Variable Names.
- Lines longer than 72 characters are strongly discouraged.
- If you write code on Windows or VSCode, please run dos2unix on all
  your Translation Units before submitting a pull request. This will
  remove the [\r\n] DOS/Windows newline terminator and replace it
  with the UNIX/Linux [\n] newline character.
