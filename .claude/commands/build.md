Build the Pina Engine project.

## Arguments
- `$ARGUMENTS`: Optional build type (debug/release). Default: debug

## Instructions

1. Parse the build type from arguments (default to "debug" if not specified)
2. Run the appropriate build command:
   - **debug**: `./build-macos.sh` or `./build-macos.sh debug`
   - **release**: `./build-macos.sh release`
3. Report build success or failure with any compiler errors
4. If successful, list the built executables in `build-macos-{type}/bin/`
