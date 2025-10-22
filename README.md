# Build Steps
* CD into the build folder
* run `./premake5 gmake`
* CD back to the root
* run `make`

## Optional steps for nvim users so that LSP can recognize raylib
* Install [bear](https://github.com/rizsotto/Bear)
* Go to project's root directory
* run `bear -- make`
* It will generate `compile_commands.json`
