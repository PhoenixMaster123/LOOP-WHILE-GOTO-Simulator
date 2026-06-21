# Contributing Guidelines

## Versioning

The versioning scheme follows the [SemVer](https://semver.org/) specification.

## Git Commit Message Convention

The commit messages should be formatted according to the guidelines by [conventionalcommits.org](https://www.conventionalcommits.org/en/v1.0.0/). The allowed commit types are:

- `feat`: Implement a feature. The `MINOR` version must be increased.
- `fix`: Fix a bug. The `PATCH` version must be increased.
- `chore`: Changes to the environment (like `.gitignore` or system integration files), but not to production code.
- `build`: Changes to build system files. This includes dependency updates.
- `docs`: Changes to documentation.
- `style`: Formatting of files without changing the meaning (like whitespaces, colons, etc.).
- `refactor`: Changes to production code without introducing new features or fixing bugs. The `PATCH` version must be increased.
- `test`: Creating and modifying test code. Production code must not be changed.

Currently there are no scope specifiers for commit messages due to the size of
the project. If the project grows in the future, scope specifiers may be
introduced.
