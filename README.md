![Demo](https://raw.githubusercontent.com/teopost/qmenu/master/qmenu.gif)

# How compile on Ubuntu


 Install ncurses library

    sudo apt-get install libncurses5-dev libncursesw5-dev
    sh ./go
    chmod 777 ./qmenu
    ./qmenu m_main.mnu

# How it works
qmenu allows to create menus for interfaces and characters in an easy and intuitive way.
Each menu in defined in a file (with extenision .mnu) that contains  the visual appearance and the definition of the items.
The file is divided oi three sections

    1. Screen Area
    2. Item Area
    3. Option Area

Each area is delimited by the characters $$ and %%



    Screen area
    $$
    Item area
    %%
    Option area

## Screen area
This area starts from the beginning of the file to the symbol $$, and contains the "drawing" of the  menu in text format.
Each menu option, to be recognized as selectable must be enclosed between the symbol ^

Example:

    ^1. Option 1^
    ^2. Option 2^

The options can be selected by using the arrow keys or by pressing the first character of the string.

In this area you can also add some variables with the following sintax:

    x[NAME_VARIABILE]


The name of the variable is composed by a prefix (x) and, the name between square brackets. These are the available values:

    * c - Centered
    * l - Left-justified
    * r - Right-justified

Note: If the symbol is put capitalize the value is shown in reverse

Variables can take the following value:

    * TIME - Current time
    * MENUNAME - Name of the menu
    * INFOTERM - Nome of the terminal
    * MNUTRACE - Path menu

Special variables:

    & - Cursor position
    ${PATH} - Displays the contents of the environment variable PATH

Example:

	C[TITLE] - Menu title centered and in reverse
	
## Item area
In this section, for each menu item, you define:

* the action to perform
* An help message
* ???

The format to use for each item is composed by 4 lines:

    1. Menu option
    2. Command to perform
    3. Help message
    4. ???


1. In the first line, insert the first character of the string which contains the menu option
2. In the second line, is possible to specify:

    * A shell command
    * A submenu to open ($menuname without extension)
    * Reserved command (return to go to the previous menu, end to quit menu) 

3. In the third line you specify the message.
This message will be shown at the variable TITLE present in the "Screen area"

Example:

    %1
    ls -lisa; pause
    This item show shell file list
    ?????


## Option area
This section contains a series of generic settings:

    TITOLO - Menu title
    RVS_ROW=0,22 - Lines displayed in reverse. In this case the line 0 and line 22
    NOKEYDISP - Disables the automatic visualization of the labels of the function keys (ET[1-10])
    ET[1-10] - Text shown to function key n if NOKEYDISP commented
    FZ[1-10] - Command (shell) for the function key n
    PW[1-10] - Password to set for the function key n
    SHELL=sh - Pressing shift-f1 starts a shell session
    EDITOR=vi - Pressing shift-f2 starts vi with the current menu
    
Each of these options can be commented out with #


# Note
Documentation in italian language is available here

* [README_it](README_it.md)

Thanks
---
* First version named appsel created by  [Giovanni Juan Oteri](https://twitter.com/giovannioteri)
* English translation of [Lorenzo Teodorani](https://github.com/teopost2)



