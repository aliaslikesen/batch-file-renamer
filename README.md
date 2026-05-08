# Batch File Renamer

A simple command-line application written in C for batch renaming files in a selected folder.

The program asks the user for:

1. The folder where the files should be renamed
2. The new base name for all files
3. The numbering convention

It then renames all files in the selected folder using indexed names.

## Features

- Rename all files in a selected folder
- Choose a custom base name
- Choose one of three numbering styles:
  - `Name1`
  - `Name-1`
  - `Name_1`
- Automatically adds an index to each file
- Preserves original file extensions
- Skips subfolders
- Works from the command line
- Written in pure C
- Cross-platform code for Windows, Linux, and macOS

## Example

Original files:

```text
image.jpg
photo.png
thumbnail.webp
```

Selected base name:

```text
PlikiMiniaturki
```

Selected numbering style:

```text
Name_1
```

Result:

```text
PlikiMiniaturki_1.jpg
PlikiMiniaturki_2.png
PlikiMiniaturki_3.webp
```

## Naming Conventions

If the base name is:

```text
File
```

The available output formats are:

```text
File1
File-1
File_1
```

The original file extension is preserved.

For example:

```text
image.jpg
```

can become:

```text
File_1.jpg
```

## Compilation

### Linux / macOS

```bash
gcc -std=c17 -Wall -Wextra -o batch-file-renamer main.c
```

### Windows using MinGW

```bash
gcc -std=c17 -Wall -Wextra -o batch-file-renamer.exe main.c
```

## Usage

Run the compiled program from the terminal.

### Linux / macOS

```bash
./batch-file-renamer
```

### Windows

```bash
batch-file-renamer.exe
```

The program will ask for the folder, the base file name, and the numbering style.

Example input:

```text
Enter the folder where the program should work: C:\Users\User\Desktop\Images
Enter the new base file name, for example PlikiMiniaturki: PlikiMiniaturki

Choose numbering convention:
1. PlikiMiniaturki1
2. PlikiMiniaturki-1
3. PlikiMiniaturki_1
Your choice: 3
```

Example output:

```text
image.jpg -> PlikiMiniaturki_1.jpg
photo.png -> PlikiMiniaturki_2.png
thumbnail.webp -> PlikiMiniaturki_3.webp

Done. Renamed 3 files.
```

## Project Structure

```text
batch-file-renamer/
├── main.c
└── README.md
```

## Requirements

- C compiler
- GCC, MinGW, Clang, or another compatible compiler

## Safety Notes

Before renaming files, it is recommended to create a backup of the folder.

The program renames files in the selected folder only. It does not rename folders or files inside subfolders.

The program uses a temporary renaming step to reduce the risk of filename conflicts during batch renaming.

## Possible Use Cases

This tool can be useful for:

- Organizing image files
- Renaming thumbnails
- Preparing files for upload
- Cleaning up downloaded files
- Giving files consistent names
- Batch-renaming assets for projects

## Repository Name Suggestion

Recommended GitHub repository name:

```text
batch-file-renamer
```

## Description for GitHub

```text
A simple C command-line tool for batch renaming files in a selected folder using custom naming conventions and indexed filenames.
```

## License

This project is open source and can be used for learning, modification, and personal projects.

You can add a specific license such as MIT if needed.
