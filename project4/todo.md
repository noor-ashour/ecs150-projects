# READ ONLY
To help debug your disk images, you will create seven small command-line utilities that access a given disk image. We have included an example disk image and expected outputs in the disk_testing directory, but make sure that your utilities can handle multiple different disk image configurations and contents.

For the `ds3ls`, `ds3bits`, and `ds3cat` utilities, you'll only need to implement the "read" calls from your local file system. These read calls are `lookup`, `stat`, and `read`. For `ds3bits` you'll want to implement `readSuperBlock`, `readInodeBitmap`, and `readDataBitmap`. We recommend getting these utilities and LocalFileSystem.cpp functions working first.

## ds3ls

The `ds3ls` prints the contents of a directory. This utility takes two arguments: the name of the disk image file to use and the path of the directory or file to list within the disk image. For directories, it prints all of the entries in the directory, sorted using the `std::sort` function. Each entry goes on its own line. For files, it prints just the information for that file. Each entry will include the inode number, a tab, the name of the entry, and finishing it off with a newline.

For all errors, we'll use a single error string: `Directory not found` printed to standard error (e.g., cerr), and your process will exit with a return code of 1. On success, your program's return code is 0.