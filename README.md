Developed a Unix shell in C
• Added internal commands and batch mode
• Implemented external commands with fork/exec
• Supported I/O redirection and background tasks (&)

## SYNOPSIS

```bash
./customshell [batch_file]
```

## STARTUP

### Build:

```bash
cd Stage1/src && make
```

### Run:

```bash
../bin/customshell          # Interactive mode
../bin/customshell commands.batch  # Batch mode
```

## BUILT-IN COMMANDS

| Command       | Action                   |
| ------------- | ------------------------ |
| `cd [dir]`    | Change/show directory    |
| `clr`         | Clear screen             |
| `dir [dir]`   | List contents (`ls -al`) |
| `environ`     | Show all env variables   |
| `echo <text>` | Print text (1 space)     |
| `help`        | Show manual              |
| `pause`       | Wait for Enter           |
| `quit`        | Exit shell               |

## BATCH MODE

Create file `test.batch`:

```text
cd /tmp
dir
quit
```

Run:

```bash
./customshell test.batch
```
