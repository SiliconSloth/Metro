## Metro Styling Conventions
These are the conventions that Metro uses for its code. They should be
followed in any additional changes or pull requests.

 - Each new command should be placed in the `commands` directory
 - Any wrapping of libgit2 functions should be done in the `gitwrapper` directory
 - Any exceptions should be placed in `error.h`
 - Includes should be put in `pch.cpp` and `pch.h` whenever possible
 - Functions should be declared in snake_case
 - Classes should be declared in CamelCase
 - All header files should have a block comment explaining the header and corresponding `.cpp` file
 - All command `.cpp` files should have a similar block comment due to the lack of a corresponding `.h` file
 - All header files should contain `#pragma` once at the start of the file
 - All methods should have multi-line documentation using the `/** */` style
 - All Git wrapped functions should have documentation corresponding to the documentation for the associated libgit2
 - If the function doesn't directly correspond to a libgit2 function, documentation should be created as for any other function based on what the function does
 - Structs whose entries are not obvious from their name should have a comment on the same line explaining them. These comments must be inline with other comments for that struct.
 - Exceptions in `error.cpp` should have a doc comment explaining when they should be used
 - Unrequired includes should not be included
 - Global namespaces should not be used. Every namespace should have a limited scope. Where appropriate specific functions or variables can be declared with `using`
 - One statement if statements should be on one line or be contained by curly braces
 - Typedef's for standard libgit types should not be used. If a libgit type has an alias such as `git::Commit`, it must be because that type is encapsulated within a class and will be fully memory managed by that class