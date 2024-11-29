#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include "StringUtils.h"
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

/*
  Use this function with std::sort for directory entries
bool compareByName(const dir_ent_t& a, const dir_ent_t& b) {
    return std::strcmp(a.name, b.name) < 0;
}
*/

void printErrorMsg() {
  cerr << "Directory not found" << endl;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << argv[0] << ": diskImageFile directory" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img /a/b" << endl;
    return 1;
  }

  // parse command line arguments
  
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  string directory = string(argv[2]);

  // Traverse from root directory inode number to directory's inode number
  int inodeNumber = fileSystem->lookup(UFS_ROOT_DIRECTORY_INODE_NUMBER, directory);
  if (inodeNumber < 0) {
    printErrorMsg();
    return 1;
  }

  // Read inode based on its number
  inode_t inode;
  if (fileSystem->stat(inodeNumber, &inode) != 0) {
    printErrorMsg();
    return 1;
  }

  // Loop through inode's direct pointers to get its files and subdirectories
  for (int i = 0; i < sizeof(inode.direct); i++) {
    int blockNum = inode.direct[i];

    // Check if this is an empty directory
    if (blockNum == 0) {
      break;
    }

    size_t entriesSize = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
    dir_ent_t *entries = new dir_ent_t[entriesSize];
    disk->readBlock(blockNum, entries);

    for (int j = 0; j < entriesSize; j++) {
      
    }
  }


  // TODO For directories, print all of the entries in the directory, sorted using std::sort
  //  (Each entry goes on its own line)


  // TODO For files, print just the information for that file.
  // Each entry will include the inode number, a tab, the name of the entry, 
  //  and finishing it off with a newline
  
  //}

  return 0;
}
