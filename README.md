# cshell

A custom Unix shell written in C, built from scratch. Supports the most commonly used shell features with a clean, readable implementation.

## Features

| Feature | Details |
|---|---|
| **Pipes** | Chain commands with `\|` (unlimited depth) |
| **I/O redirection** | `<` `>` `>>` |
| **Background jobs** | Trailing `&` |
| **Built-ins** | `cd`, `pwd`, `history`, `help`, `exit` |
| **Command history** | Last 100 commands, browseable with readline arrow keys |
| **Quoting** | Single and double quote handling |
| **Signal handling** | Ctrl-C interrupts child, not the shell |

## Build

```bash
# macOS (readline via Homebrew)
brew install readline
make

# Linux
sudo apt install libreadline-dev
make
```

## Usage

```
$ make run
./cshell

# Pipes
ls -la | grep ".c" | wc -l

# Redirection
sort < words.txt > sorted.txt
cat log.txt >> combined.txt

# Background
sleep 10 &

# Built-ins
cd ~/projects
history
help
exit 0
```

## Architecture

```
├── include/
│   └── shell.h        # All structs, constants, and function declarations
├── src/
│   ├── main.c         # REPL loop, readline integration, signal setup
│   ├── parser.c       # Tokenizer → Pipeline struct (handles quotes, operators)
│   ├── executor.c     # fork/exec, pipe setup, I/O redirection, waitpid
│   └── builtins.c     # cd, pwd, history, help, exit + history buffer
└── Makefile
```

## Requirements

- GCC (C11)
- libreadline (`brew install readline` / `apt install libreadline-dev`)
- macOS or Linux

## License

MIT
