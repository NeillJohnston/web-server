# Project Structure

## Source Code

The root of all source is `src/`. Server source is in `src/server`.

Each "module" gets its own folder. Modules comprise many units, and can be split up into multiple files if it makes sense to do so.

## Unit Tests

All unit tests should be written with Underscore. The files can be called anything as long as they are preceded with an underscore and appear in the directory for the C file(s) they pertain to.

Unit tests should be written before units are developed.

If tests can't be written because a unit encapsulates too much behavior to create tests for, then go back to designing and split the unit up. This will reduce complexity and make the code more bearable to come back to later, since 1. there's less to read at a time and 2. it can be adequately tested to prove it works.

## Output

The root of all output is `out/`.

The server is a single binary executable named `server`.

The website files go to `out/public/`.