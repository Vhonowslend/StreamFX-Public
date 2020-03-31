# Commits
Commits should always only focus on a single change that is necessary for that commit to work. For example, a commit that changes how something logs messages should not also include a new blur effect. In those cases, the commit should be split up into two, so that they can be reverted independently from another.

## Commit Subject
The subject line of a commit should begin with the prefix followed by a `: `, and then followed by a summary of what the change does, which should be no longer than 52 alphanumerical characters including whitespace. The prefix is determined by the file being modified, simply remove the extension or find the group that a file belongs to. For example, a modifiation to blur.effect would have the category effects, due to it being re-usable.

### Prefixes
- effects: Anything modifying generic effects like blur.effect, color-conversion.effect, mask.effect, etc.
- locale: Changes in `/data/locale`.
- examples: Changes in `/data/examples` that are not directly influenced by a change to one of the filters, sources or transitions.
- project: Changes to files like README, CONTRIBUTING, AUTHORS, etc.
- cmake: Changes to CMake build scripts.
- ci: Changes to Continuous Integration.

All other files should be prefixed with the main file changed, so a change to the translations for Source Mirror would be `source-mirror: commit`.

## Commit Messages
The commit message should always convey why this change was necessary, what is being changed and how it affects the code when being run. There are rare cases where this can be left out (formatting, refactoring, ...) but it should always be descriptive of what is actually being done.

# Coding Guidelines

## Naming
The project uses the generally known snake_case for code and the uppercase variant for enumerations and macros:

### Macros (ELEPHANT_CASE)
- Casing: Uppercase
- Separator: `_`
- Prefixes: `S_` for global values, `ST_` for local (this file) values, `D_` for simple functions, `P_` for complex functions
- Suffixes: No

Example:
```
#define S_PI 3.14141
#define ST_PI2 S_PI / 2.0
#define D_(x) S_PI * x
#define P_(x, y) double_t x(double_t a, double_t b) { return a * b * y; }
```

### Enumerations (snake_case)
- Casing: Lowercase
- Separator: `_`

Example:
```
enum my_enum {};
enum class my_enum_class {};
enum class my_enum_class_int : int {};
```

#### Enumeration Entries (ELEPHANT_CASE)
- Casing: Uppercase
- Separator: `_`

Example:
```
enum my_enum {
	ENTRY_1,
	ENTRY_2
};
```

### Variables (snake_case)
- Casing: Lowercase
- Separator: `_`
- Prefixes: No
- Suffixes: No

Example:
```
int my_var = 0;
```

### Functions (snake_case)
- Casing: Lowercase
- Separator: `_`
- Prefixes: No
- Suffixes: No (differentiate by parameters only)

Example:
```
// This is forbidden.
void func();
int func_int();

// This is okay.
void func();
void func(int& result);
```

### Namespaces (snake_case)
- Casing: Lowercase
- Separator: `_`

Example:
```
namespace a_space {};
```

### Classes, Structs, Unions (snake_case)
- Casing: Lowercase
- Separator: `_`
- Prefixes: No
- Suffixes: No

Example:
```
class a_class {};
class interface_class {};
```

#### Interface Classes
Interface Classes are handled like normal classes. There are no prefixes or suffixes to attach.

#### Methods (snake_case)
- Casing: Lowercase
- Separator: `_`
- Prefixes: No
- Suffixes: No (differentiate by parameters only)

Example:
```
class a_class {	
	// This is forbidden.
	void func();
	int func_int();
	
	// This is okay.
	void func();
	void func(int& result);
};
```

#### Member Variables (snake_case)
- Casing: Lowercase
- Separator: `_`
- Prefixes: `_` if private, otherwise none
- Suffixes: No

Example:
```
class a_class {
	int64_t _local_var;
	void* _pointer;
	int32_t _id;
};
```

### Type Definitions (snake_case)
- Casing: Lowercase
- Separator: `_`
- Prefixes: No
- Suffixes: `_t`

Example:
```
typedef int32_t my_type_t;
```

## Preprocessor Macros
Preprocessor `#define` Macros should be used sparingly, due to their nature of changing code before the compiler gets a chance to work with it. Unless necessary, they should never be in a header file.

