ncdc -- an ncurses discord client

# Description

`ncdc` is a command line, ncurses based discord client, similar to how
cmus is a ncurses music viewer. In fact, both cmus and irssi were huge
inspirations for this project.

# Requirements

There are no binary packages of yet, so you will have to make due with
compiling it yourself. You will need the following libraries, and their
development files (depending on your distribution):

* gcc
* make
* cmake
* libcurl
* libglib2
* libconfuse
* libjansson
* libevent2
* libncursesw
* libpanel

# Building & Installing

Use `cmake` to build and install ncdc:

```shell
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
$ make
$ sudo make install
```

This will build, and install `ncdc` into your `/usr/bin` for your using
pleasure.

# Configuration

The configuration file of `ncdc` lies within `$HOME/.config/ncdc` and is
called `config`. There is not much there yet, except it allows you to
specify an account to login with:

```
account "someaccount" {
    email = "somewhere@example.com"
    password = "YourPasswordInPlainText"
}
```

Since the password is there in plain text it never hurts to make sure
that the file has proper permissions.

# Using

There are three input panes in the view. To the left is guild overview,
showing you all of your guilds, and their channels. To the right is the
current text view showing you the contents of the text clients you have
open. At the bottom is the command input line, allowing you to set off
commands.

The text view has one standard text view channel called "status" in which
ncdc will print errors, informational messages (such as friends lists),
and perhaps the odd event that has occoured.

## Hot keys

| Hotkey      | Function              | Usable in view |
|-------------|-----------------------|----------------|
| F1          | switch to input       | globally       |
| F2          | switch to guild view  | globally       |
| F3          | switch to chat view   | globally       |
| ALT+LEFT    | previous text view    | globally       |
| ALT+RIGHT   | next text view        | globally       |
| LEFT        | collapse tree item    | guilds         |
| RIGHT       | expand tree item      | guilds         |
| UP          | previous tree item    | guilds         |
| DOWN        | next tree item        | guilds         |
| ENTER       | open selected channel | guilds         |
| LEFT, RIGHT | move cursor           | input          |
| C-w         | kill word left        | input          |
| C-u         | kill all left         | input          |
| C-k         | kill all right        | input          |
| C-b         | cursor backwards      | input          |
| C-f         | cursor forwards       | input          |
| C-d         | delete at position    | input          |
| BACKSPACE   | delete backwards      | input          |

## Commands

These commands can be issued in the input line, and allow you to interface
with ncdc more easily. You can quote arguments with a double quote `"` in
case an argument for a command contains spaces, or special characters. If
you don't start your line with a forward slash `/` then the text is seen
as a chat line, and sent to the appropriate channel. Arguments, such as
guild names, or user names are all case sensitive.

| Command        | Function                          | Arguments                         | Notes |
|----------------|-----------------------------------|-----------------------------------|-------|
| /ack           | Mark channel as read              |                                   | WIP   |
| /connect       | Connect as the given account      | account, as named in config       |       |
| /friend        | List all friends                  |                                   |       |
| /friend accept | Accept a friend request           | full discord name, i.e. name#XXXX |       |
| /friend add    | Add a friend                      | full discord name, i.e. name#XXXX |       |
| /friend list   | List all friends                  |                                   |       |
| /friend remove | Remove a friend                   | full discord name, i.e. name#XXXX |       |
| /friends       | Alias for "/friend"               |                                   |       |
| /join          | Join a guild channel              | "guild name" "channel name"       |       |
| /login         | Alias for /connect                |                                   |       |
| /logout        | Log current user out              |                                   |       |
| /markread      | Alias for /ack                    |                                   | WIP   |
| /msg           | Private message a friend          | full discord name, i.e. name#XXXX |       |
| /post          | Post a message to current channel | full message to post              |       |
| /quit          | Exit, and quit                    |                                   |       |

## Work In Progress

This client is very much work in progress, and lacks a lot of features. Here
are a few that would be nice to have:

* auto completion
* man pages
* closing of chat windows
* notification handling if guilds are edited
* notification handling if friends send you a message
* /markread support
* handling for markdown, i.e. bold, italics, colours etc.
* voice chat support
* better performing websocket implementation

## Hacking

The code is written in C, and in C only. Use four spaces, no tabs. No
tabs.  and indent according to K&R C. Use ref counting wherever
possible, and put on a condom when handling pointers. No tabs. Use the
glib over rolling your own arrays, hash tables, or other data
structures. Use `stdint` over glib basic types, if absolute necessary.
Stick to standard types (i.e. `int`) if the size doesn't matter.

Stick pure discord functionality in `libdc` (libdiscord), such as JSON
parsing, web socket action, API calls, among others. `libdc` uses single
byte strings (i.e. UTF-8). Put all UI stuff into `ncdc` (ncurses discord
client), and use `wchar_t` there for displaying purposes, along with the
wide character functions of ncurses.

Happy Hacking!
