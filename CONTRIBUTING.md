# Contributing
This document intends to teach you the proper way to contribute to the project as a set of guidelines. While they aren't always enforced, your chances of your code being accepted are significantly higher when you follow these. For smaller changes, we might opt to squash your changes to apply the guidelines below to your contribution.

<details open><summary><h2 style="display: inline-block;">Repository & Commits</h2></summary>

As this is a rather large project, we have certain rules to follow when contributing via git.

### Linear History
We follow the paradigm of linear history which forbids branches from being merged, thus changes made on branches are `git rebase`d back onto the root. This simplifies the code history significantly, but makes reverting changes more difficult.

❌ `git merge`  
✅ `git rebase`

### Commits
A commit should be containing a single change, even if it spans multiple units, and has the following format:

```
prefix: short description

optional long description
```

The short description should be no longer than 120 characters and focus on the important things. The long description is optional, but should be included for larger changes.

#### The appropriate `prefix`

<table>
	<tr>
		<th>Path(s)</th>
		<th>Prefix</th>
		<th>Example</th>
	</tr>
	<tr>
		<td>
			data/locale
		</td>
		<td>locale</td>
		<td>
			<code>data/locale/en-US.ini</code> -> <code>locale</code>
		</td>
	</tr>
	<tr>
		<td>components/name</td>
		<td>name</td>
		<td>
			<code>components/shader</code> -> <code>shader</code>
		</td>
	</tr>
	<tr>
		<td>
			source<br>
			templates<br>
			data<br>
			ui
		</td>
		<td>core</td>
		<td>
			<code>ui/main.ui</code> -> <code>core</code>
		</td>
	</tr>
	<tr>
		<td>Anything else</td>
		<td><b>Omit the prefix</b></td>
		<td></td>
	</tr>
</table>

If multiple match, apply the prefix that changes the most files. If all are equal, alphabetically sort the prefixes and list comma separated.

</details>


<details open><summary><h2 style="display: inline-block;">Coding</h2></summary>

### Documentation
The short form of the this part is **Code != Documentation**. Documentation is what you intend your Code to do, while Code is what it actually does. If your Code mismatches the Documentation, it is time to fix the Code, unless the change is a new addition in terms of behavior or functionality. Note that by this we don't mean to document things like `1 + 1` but instead things like the following:

```c++
int32_t idepth         = static_cast<int32_t>(depth);
int32_t size           = static_cast<int32_t>(pow(2l, idepth));
int32_t grid_size      = static_cast<int32_t>(pow(2l, (idepth / 2)));
int32_t container_size = static_cast<int32_t>(pow(2l, (idepth + (idepth / 2))));
```

```c++
class magic_class {
	void do_magic_thing(float magic_number) {
		// Lots and lots of SIMD code that does a magic thing...
	}
}
```

Documenting what a block of Code does not only helps you, it also helps other contributors understand what this Code is supposed to do. While you may be able to read your own Code (at least for now), there is no guarantee that either you or someone else will be able to read it in the future. Not only that, but it makes spotting mistakes and fixing them easier, since we have Documentation to tell us what it is supposed to do!

### Naming & Casing
The project isn't too strict about variable naming as well as casing, but we do prefer a universal style across all code. While this may appear as removing your individuality from the code, it ultimately serves the purpose of making it easier to jump from one block of code to the other, without having to guess at what this code now does.

Additionally we prefer it when things are named by what they either do or what they contain, instead of having the entire alphabet spelled out in different arrangements. While it is fine to have chaos in your own Code for your private or hobby projects, it is not fine to submit such code to other projects.

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

</details>

<details open><summary><h2 style="display: inline-block;">Localization</h2></summary>

We use Crowdin to handle translations into many languages, and you can join the [StreamFX project on Crowdin](https://crowdin.com/project/obs-stream-effects) if you are interested in improving the translations to your native tongue. As Crowdin handles all other languages, Pull Requests therefore should only include changes to `en-US.ini`.

</details>

## Further Resources
- A guide on how to build the project is in BUILDING.MD.
- A no bullshit guide to `git`: https://rogerdudler.github.io/git-guide/
    - Remember, `git` has help pages for all commands - run `git <command> --help`.
	- ... or use visual clients, like TortoiseGit, Github Desktop, SourceTree, and similar. It's what I do.
