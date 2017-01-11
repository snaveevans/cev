## Description

**cev** is a terminal application to output the keycode associated with a keypress.

**cev** must have access to the *Accessibility API*, or be ran as root.

secure keyboard entry must be disabled for **cev** to receive key-events.

## Install

a binary release is available on Github and through Homebrew
```
brew install koekeishiya/homebrew-formulae/cev
```

## Usage

run in terminal as root, and start hitting keys.
because **cev** captures key-input, `ctrl-c` must be used to quit!

## Development

build *cev*
```
make install # optimized build
make         # debug build
```
