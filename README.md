# A sample Magnum + wxWidgets C++ project

Those packages and tools are used to build this project.

[mosra/magnum: Lightweight and modular C++11 graphics middleware for games and data visualization](https://github.com/mosra/magnum)

[wxWidgets/wxWidgets: Cross-Platform GUI Library - Report issues here: https://trac.wxwidgets.org/](https://github.com/wxWidgets/wxWidgets)

[MSYS2](https://www.msys2.org/)

[eranif/wx-config-msys2: wx-config tool for MSYS2 based installation of wxWidgets using the mingw64 repository](https://github.com/eranif/wx-config-msys2)

[Code::Blocks - Code::Blocks](https://www.codeblocks.org/)

# How to build

First, you have [MSYS2](https://www.msys2.org/) installed in your Windows.

The wxWidgets library could be installed by such commands in the msys2 shell.

~~~~
pacman -S mingw-w64-x86_64-wxmsw3.1
~~~~

And the Magnum library should also be installed by the commands

~~~~
pacman -S mingw-w64-x86_64-magnum
~~~~

In the root folder, ther is a file named `wx-config-msys2.exe`, which I built from the project: [eranif/wx-config-msys2: wx-config tool for MSYS2 based installation of wxWidgets using the mingw64 repository](https://github.com/eranif/wx-config-msys2). The `wx-config-msys2.exe` will be called when you click the "Build" button in Code::Blocks to generate the correct build options.





