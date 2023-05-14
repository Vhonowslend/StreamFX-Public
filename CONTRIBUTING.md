# Contributing
This document goes over how you (and/or your organization) are expected to contribute. These guidelines are softly enforced and sometimes not required.

## Localization
We use Crowdin to handle translations into many languages, and you can join the [StreamFX project on Crowdin](https://crowdin.com/project/obs-stream-effects) if you are interested in improving the translations to your native tongue. As Crowdin handles all other languages, Pull Requests therefore should only include changes to `en-US.ini`.

## Commit Guidelines
Commits should focus on a single change such as formatting, fixing a bug, a warning across the code, and similar things. This means that you should not include a fix to color format handling in a commit that implements a new encoder, or include a fix to a bug with a fix to a warning.

### Linear History
This project prefers the linear history of `git rebase` and forbids merge commits. This allows all branches to be a single line back to the root, unless viewed as a whole where it becomes a tree. If you are working on a branch for a feature, bug or other thing, you should know how to rebase back onto the main branch before making a pull request.

### Commit Message & Title
We require a commit message format like this:

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
- `/patches` -> `patches`
- `/tools` -> `tools`
- `/ui` -> `ui` (if not part of a `code` change)
- Most other files -> `project`

If multiple locations match, they should be alphabetically sorted and separated by `, `. A change to both `ui` and `code` will as such result in a prefix of `code, ui`. If a `code` change only affects a single file, or multiple files with a common parent file, the prefix should be the path of the file, like shown in the following examples:

- `/source/encoders/encoder-ffmpeg` -> `encoder/ffmpeg`
- `/source/filters/filter-shader` -> `filter/shader`
- `/source/encoders/handlers/handler`, `/source/encoders/encoder-ffmpeg` -> `encoder/ffmpeg`

## Coding Guidelines

### Documentation
Documentation should be present in areas where it would save time to new developers, and in areas where an API is defined. This means that you should not provide documentation for things like `1 + 1`, but for things like the following:

```c++
int32_t idepth         = static_cast<int32_t>(depth);
int32_t size           = static_cast<int32_t>(pow(2l, idepth));
int32_t grid_size      = static_cast<int32_t>(pow(2l, (idepth / 2)));
int32_t container_size = static_cast<int32_t>(pow(2l, (idepth + (idepth / 2))));
```

```c++
class magic_class {
	void do_magic_thing(float magic_number);
}
```

Both of these examples would be much easier to understand if they had proper documentation, and save hours if not even days of delving into code. Documentation is about saving time to new developers, and can't be replaced by code. Code is not Documentation!

### Naming & Casing
All long-term objects should have a descriptive name, which can be used by other developers to know what it is for. Temporary objects should also have some information, but do not necessarily follow the same rules.

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
	- private: `_`
	- protected, public: None
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

	public:
	float exa_mple; // ✔

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

### Global Variables
Only allowed if there is no other cleaner way to handle this.

### Classes
Special rules for `class`

#### Members
All class members must be `private` and only accessible through get-/setters. The setter of a member should also validate if the setting is within an allowed range, and throw exceptions if an error occurs. If there is no better option, it is allowed to delay validation until a common function is called.

## Building
Please read [the guide on the wiki](https://github.com/Xaymar/obs-StreamFX/wiki/Building) for building the project.

