# Contributing to Rigging Tools

- [Code of Conduct](#code-of-conduct)
- [Contributing Changes](#contributing-changes)
- [Reporting issues](#reporting-issues)
- [Feature Requests](#feature-requests)
- [Submitting Changes](#submitting-changes)
  - [Unit test](#unit-tests)
  - [Code Style](#code-style)
    - [Naming Style](#naming-style)
- [Patch Submission](#patch-submission)
  - [Commit Message Format](#commit-message-format)

## Code of Conduct
This project is released with a [Contributor Code of
Conduct](/CODE_OF_CONDUCT.md). By participating in this project you
agree to abide by its terms.

## Reporting issues
Security issues and general (non-security) issues are reported differently.

### General issues
Most issues fall into this category and are not security related, such as build errors, runtime errors, etc.
Please report all general issues through a GitHub [issue](https://github.com/intel/riggingtools/issues). TODO: Need a template here

### Security issues
Please see the [Security Policy](SECURITY.md) for details on how to report security issues

## Feature Requests
Feature requests use the same 'issue' mechanism as general issues. Create a new GitHub [feature request](https://github.com/intel/riggingtools/issues). TODO: Need a template here

## Submitting Changes
Please ensure contributions do not introduce regressions, and conform to the programming conventions prior to submission, as described below.

### CI/CD
CI/CD is one of our goals], for now just make sure the unit test pass manually before creating a pull request.

### Unit Tests
Unit tests are provided for tools, but currently no single mechanism exists to test all of them at once (WIP as part of CI/CD)

### Code Coverage
(WIP as part of CI/CD)

### Code Style
There are 7 different programming languages used in Rigging Tools at the time of writing this (more if you count CMake and similar tools).
Instead of some monolithic document trying to cover all the cases, follow the "When in Rome" principle:
> your code should look the same as what's already there

That should cover most of the cases.
Simple example: C++ opening `{` are always on their own line, while swift has it at the end of the line, and _you know this_ by looking at existing C/C++ and swift code in this repository.
Pay attention to contextual awareness and you'll be good-to-go.

That said, this code ain't perfect and their may be discrepencies; in these cases pick what you think is best and discuss in the pull request comments.

#### Naming style
In short, be _concise_, not brief. Text is cheap, knowledge is expensive - there is value in learning the art of concision.

This is not good for readability and is hard on the brain:

`int bm=0`

What is `bm`? The author communicates this in one of two ways:
 - adds _more_ text in the form of comments, probably negating the quick-n'-simple mindset that lead to this useless name in the first place
 - forces every other developer to infer what on earth 'bm' means from subsequent behavior that is oh-so-obvious to the author but merely the second and 13th letters of the alphabet (respectively) to everyone else

Consider this alternative instead:

`int boundaryMax=0`

...and breathe in a breath of fresh air knowing you have potentially saved others from wasting their precious brain cycles. All for the low cost of 9 additional characters.

#### Comments
Comments are good...if they add value. Mindlessly adding a 15-line file-level comment with completely redundant information is not adding value - it's adding noise.
Here is a table with some guidelines for commenting:

| Comment scope | Frequency | Guideline |
| ------------- | --------- | --------- |
| Function contents | Often | Use comments inside functions so that your function read like a story, allowing yourself and others to naturally follow the flow of logic |
| Utility classes | Sometimes | Utility classes may need a little blurb about the kind of utility they provide |
| Interfaces | Sometimes | Interfaces may benefit from this more than data structs and derrived classes |
| Functions | Sometimes | Try to avoid this by good function naming and attributes |
| Files | Rarely | Interfaces and utility files may benefit from file-level comments, but generally your filename + content should be self-evident. If not, check your architecture |
| Subclasses | Rarely | the purpose of a subclass is usually inferred from its name and the interface(s) it implements |
| Data structs | Rarely | Struct name and members should be self evident |
| Enums | Rarely | You're probably doing something strange indeed to document an enum |

### Patch Submission
Patch submission = [pull request](https://help.github.com/en/articles/about-pull-requests).
Pull requests should be associated with a bug report or feature request, making sure to [reference the issue](https://help.github.com/en/articles/closing-issues-using-keywords) number in the pull request commit message.

As much as possible, a patch should contain a single logical change. Avoid mixing unrelated changes into the patch.

#### Commit Message Format
Git commit messages should follow the commonly used 50/72 rule:
```
Summary line no longer than 50 characters wide.

Optional detailed description of the changes in the commit, wrapped to
be no more than 72 columns wide.
```
There is not need to add a [`Signed-off-by`](https://git.wiki.kernel.org/index.php/CommitMessageConventions) trailer to the commit message because it isn't used.
