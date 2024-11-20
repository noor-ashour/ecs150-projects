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
  
  // TODO perhaps put this in a loop, inodeNumber starts at 0 and changes w/ every traversal
  // Look up directory/file and return its inode number
  int inodeNumber = fileSystem->lookup(0, directory);
  if (inodeNumber != 0) {
	  cerr << "Directory not found" << endl;
	  return 1;
  }

  // TODO For directories, print all of the entries in the directory, sorted using std::sort
  //  (Each entry goes on its own line)

  // TODO For files, print just the information for that file.
  // Each entry will include the inode number, a tab, the name of the entry, 
  //  and finishing it off with a newline
  
  return 0;
}
