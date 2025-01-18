# source files for `smm.h`

The `*.hpp` files in this directory generate `smm.h`. They are assembled in alphabetical order; for this reason, they are prefixed with a double-digit number to ensure the correct ordering.

## source file internal organization

Within each source file, there are sections delineated by special comments. Each section comment is of the form `/* @[SECTION TITLE] */`. A source file may optionally have any of the following sections:

  * `@INCLUDE`
  * `@HEADER`
  * `@EXTERN`
  * `@IMPLEMENTATION`
  * `@SETUP`
 
Lines within the `@INCLUDE` section are added to a set and then inserted at the top of the document. This allows each file to declare the includes that it needs but inserts the needed includes only once.

Code in the `@HEADER` section is inserted after the includes and inside the `smm` namespace. **Source files do not need to wrap their header sections in the smm namespace.**

Code in the `@EXTERN` section is inserted after the `@HEADER` section and is outside of the `smm` namespace. It is intended for doing things like declaring global extern variables.

Code in the `@IMPLEMENTATION` section comes last and is surrounded by a `#ifndef SMM_IMPLEMENTATION` macro guard.

The final element of the implementation section is a definition for `void smm::setup()`, which contains lines from each `@SETUP` section.

Each source file must end with `/* @END */` for the section-matching regexes to work correctly.
