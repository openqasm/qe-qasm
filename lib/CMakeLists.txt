include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckCXXSymbolExists)

list(APPEND REQUIRED_FEATURES
  cxx_std_17
  cxx_auto_type
  cxx_decltype
  cxx_default_function_template_args
  cxx_defaulted_functions
  cxx_delegating_constructors
  cxx_enum_forward_declarations
  cxx_explicit_conversions
  cxx_extended_friend_declarations
  cxx_extern_templates
  cxx_final
  cxx_lambdas
  cxx_long_long_type
  cxx_nullptr
  cxx_override
  cxx_range_for
  cxx_raw_string_literals
  cxx_right_angle_brackets
  cxx_rvalue_references
  cxx_static_assert
  cxx_strong_enums
  cxx_trailing_return_types
  cxx_uniform_initialization
  cxx_variadic_macros
  cxx_variadic_templates
)

add_subdirectory(Parser)
add_subdirectory(AST)
add_subdirectory(Frontend)
add_subdirectory(Diagnostic)
