# Manual

## Compile the source code

- Clone the source code from `git clone https://sourceware.org/git/valgrind.git`

- Go to the source code repository `valgrind` folder

- Run the following commands

```
./autogen.sh
./configure --prefix=/...
make
make install
```

## Writing our Valgrind tool



- Choose a name for the tool. We will use `foobar`

- Make three new directories `foobar/`, `foobar/docs/` and `foobar/tests/`

- Create new empty file `foobar/test/Makefile.am`

- Copy `none/Makefile.am` into `foobar/`. Replace `"none"`,`"NONE"`, `"nl"`, `"nl-"` with `"foobar"`,`"FOOBAR"`, `"fl_"`, `"fl-"` respectively

- Copy `none/nl_main.c` into `foobar/`. Edit the *details* lines in `nl_pre_clo_init` and replace `"nl_"` throughout with `"fl_"`

- Edit `Makefile.am` in Valgrind directory, adding `foobar` to the `TOOLS` and `EXP_TOOLS` variables

- Edit `configure.ac`, adding `foobar/Makefile` and `foobar/tests/Makefile` to `AC_CONFIG_FILES` list

- Run

```
./autogen.sh
./configure --prefix=`pwd`/inst
make
make install
```

