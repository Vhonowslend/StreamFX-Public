# Contributing
This document goes over how you are expected to contribute to the project. This document is softly enforced, and considered a guideline instead of a requirement.

## Localization
We use Crowdin to handle translations into many languages. Please join the [StreamFX project on Crowdin](https://crowdin.com/project/obs-stream-effects) if you are interested in improving the translations to your native tongue. Pull Requests therefore should only include changes to `en-US.ini`, and no other localization files should be touched.

## Commit Guidelines
Commits should focus on a single change, such as formatting, fixing a single bug, a single warning across the code, and similar things. This means that you should not include a fix to color format handling in a commit that implements a new encoder.

### Linear History
This project prefers the linear history of `git rebase`, and as such forbids merge commits. This allows all branches to be a single line back to the root, unless viewed as a whole where it becomes a tree. If you are working on a branch for a feature, bug or other thing, you should learn how to rebase back onto the main branch before making a pull request.

### Commit Message & Title
As the StreamFX project uses linear history without merge commits, we require a commit message format like this:

```
prefix: short description

optional long description
```

The `short description` should be no longer than 80 characters, excluding the `prefix: ` part. The `optional long description` should be present if the change is not immediately obvious - however it does not replace proper documentation.

#### The correct `prefix`
Depending on where the file is that you ended up modifying, or if you modified multiple files at once, the prefix changes. Take a look at the list to understand which directories cause which prefix:

- `/CMakeLists.txt`, `/cmake` -> `cmake`
- `/.github/workflows` -> `ci`
- `/data/locale`, `/crowdin.yml` -> `locale`
- `/data/examples` -> `examples`
- `/data` -> `data` (if not part of another prefix)
- `/media` -> `media`
- `/source`, `/include` -> `code`
- `/templates` -> `templates` (or merge with `cmake`)
- `/third-party` -> `third-party`
- `/tools` -> `tools`
- `/ui` -> `ui` (if not part of a `code` change)
- Most other files -> `project`

Multiple prefixes should be separated by `, ` and sorted alphabetically so that a change to `ui` and `code` results in a prefix of `code, ui`. If only a single code file was changed or multiple related file with a common parent were changed, the `code` prefix should be replaced by the path to the file like in these examples:

- `/source/encoders/encoder-ffmpeg` -> `encoder/ffmpeg`
- `/source/filters/filter-shader` -> `filter/shader`
- and so on.

These guidelines are soft requirements and may be extended in the future.

## Coding Guidelines

### Documentation & Comments
Your code should contain the comments where they would save time, as well as documentation for "public" facing functionality. This means that you shouldn't explain things like `1 + 1`, but should provide explanations for complex things. Consider the following:

```c++
int32_t idepth         = static_cast<int32_t>(depth);
int32_t size           = static_cast<int32_t>(pow(2l, idepth));
int32_t grid_size      = static_cast<int32_t>(pow(2l, (idepth / 2)));
int32_t container_size = static_cast<int32_t>(pow(2l, (idepth + (idepth / 2))));
```

This would be much easier to read if it had a an explaination and didn't require parsing the math. Comments are all about saving time to developers not already invested into the code.

### Naming & Casing
Names for anything should describe the purpose or function of the thing, unless the thing is temporary such as iterators.

#### Macros
- Casing: ELEPHANT_CASE
- Separator: `_`
- Prefixes: Optional
	- `S_` for global values
	- `ST_` for local values
	- `D_` for simple functions
	- `P_` for complex functions
- Suffixes: No

##### Example
```c++
#define EXAMPLE FALSE // ❌
#define S_PI 3.14141 // ✔ (in .h and .hpp files)
#define ST_PI2 S_PI / 2.0 // ✔
#define D_(x) S_PI * x // ✔
#define P_(x, y) double_t x(double_t a, double_t b) { return a * b * y; } // ✔
```

#### Namespaces
- Casing: snake_case
- Separator: `_`
- Prefixes: None
- Suffixes: None

##### Example
```c++
namespace BLA {}; // ❌
namespace a_space {}; // ✔
```

#### Type Definitions
- Casing: snake_case
- Separator: `_`
- Prefixes: No
- Suffixes: `_t`

Example:
```c++
typedef int32_t my_type; // ❌
typedef int32_t my_type_t; // ✔
```

#### Enumerations
- Casing: snake_case
- Separator: `_`
- Prefixes: No
- Suffixes: No

##### Entries
- Casing: ELEPHANT_CASE
- Separator: `_`
- Prefixes: Conditional
	- `enum`: `STREAMFX_<ENUM_NAME>_`
	- `enum class`: None
- Suffixes: No

##### Example
```c++
enum my_enum { // ✔
	STREAMFX_MY_ENUM_ENTRY_1, // ✔
	ENUM_ENTRY_2 // ❌
};
enum class my_enum : int { // ✔
	STREAMFX_MY_ENTRY_1, // ❌
	ENUM_ENTRY_2 // ✔
};
enum class my_enum_int : int { // ❌, has `_int` suffix.
};
```

#### Variables
- Casing: snake_case
- Separator: `_`
- Prefixes:
	- Locals: None
	- Globals: `g_`
- Suffixes: None

##### Example
```c++
float example; // ❌
float g_example; // ✔

function example() {
	float _example; // ❌
	float example; // ✔
}
```

#### Classes & Structures
- Casing: snake_case
- Separator: `_`
- Prefixes: None
- Suffixes: None

##### Members
- Casing: snake_case
- Separator: `_`
- Prefixes:
	- `_` for private, protected
	- None for public (prefer set-/get-ters)
- Suffixes: None

##### Methods
- Casing: snake_case
- Separator: `_`
- Prefixes: None
- Suffixes: None

##### Example
```c++
class example {
	float example; // ❌
	float _example; // ✔

	int example_int(); // ❌, has `_int` suffix.
	void example(); // ✔
	void example(int& result); // ✔
}

struct example {
	float _example; // ❌
	float example; // ✔
}
```

#### Unions
- Casing: snake_case
- Separator: `_`
- Prefixes: None
- Suffixes: None

##### Union Members
- Casing: snake_case
- Separator: `_`
- Prefixes: None
- Suffixes: None

##### Example
```c++
union {
	float _example; // ❌
	float example; // ✔
}
```

#### Functions
- Casing: snake_case
- Separator: `_`
- Prefixes: No
- Suffixes: No

##### Example
```c++
void func(); // ✔
void func(int& result); // ✔
int func_int(); // ❌
```

#### Interface Classes
Interface Classes are handled like normal classes. There are no prefixes or suffixes to attach.

### Preprocessor Macros
Pre-processor Macros are a "last stand" option, when all other options fail or would produce worse results. If possible and cleaner to do so, prefer the use of `constexpr` code.

### Classes
#### Members
Members of classes should be private and only accessible via get/set methods.

## Building
Please read [the guide on the wiki](https://github.com/Xaymar/obs-StreamFX/wiki/Building) for building the project.

