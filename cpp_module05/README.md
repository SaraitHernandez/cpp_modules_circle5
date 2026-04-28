# C++ Module 05 — Repetition and Exceptions

School 42 project: bureaucrats, forms, abstract forms, and an intern factory.  
Standard: **C++98**. Compiler: `c++` with **`-Wall -Wextra -Werror`**.

## Author

- **Login:** sarherna  
- **Email:** sarherna@student.42.fr  

## Requirements

- `make`
- A C++ compiler supporting **`-std=c++98`**

## Repository layout

| Directory | Content |
|-----------|---------|
| `ex00/` | `Bureaucrat` and grade exceptions |
| `ex01/` | `Form`, signing, `Bureaucrat::signForm` |
| `ex02/` | Abstract `AForm`, concrete forms, `execute` / `executeForm` |
| `ex03/` | Same as `ex02` plus `Intern::makeForm` |

## Compilation

From the root of the repository:

```bash
cd ex00 && make
cd ../ex01 && make
cd ../ex02 && make
cd ../ex03 && make
```

Or rebuild from scratch:

```bash
cd ex00 && make re
# same for ex01, ex02, ex03
```

## Usage

After `make`, each exercise produces a binary in its directory:

| Exercise | Binary   | Run |
|----------|----------|-----|
| `ex00`   | `bureaucrat` | `./bureaucrat` |
| `ex01`   | `form`       | `./form` |
| `ex02`   | `aform`        | `./aform` |
| `ex03`   | `intern`       | `./intern` |

**Note:** `ex02` / `ex03` may create files in the working directory (e.g. `*_shrubbery` for shrubbery forms).

## Clean

```bash
make fclean
```

Run inside each `ex00` … `ex03` directory.

## Subject summary (by exercise)

- **ex00 — Bureaucrat:** constant name, grade 1–150, increment/decrement, `GradeTooHighException` / `GradeTooLowException`, `operator<<`.
- **ex01 — Form:** constant name, sign/exec grades, `beSigned(Bureaucrat)`, `Bureaucrat::signForm`, same grade rules and exceptions.
- **ex02 — AForm:** abstract base, concrete `ShrubberyCreationForm`, `RobotomyRequestForm`, `PresidentialPardonForm`; `execute` checks signed + executor grade; `Bureaucrat::executeForm`.
- **ex03 — Intern:** `makeForm(name, target)` returns `AForm*` (heap); unknown form names are reported without a huge `if/else` chain.

## Evaluation

- Compile with the flags above; code must remain valid with **`-std=c++98`**.
- No STL containers/algorithms where the subject forbids them (this module follows the general 42 C++ module rules).
