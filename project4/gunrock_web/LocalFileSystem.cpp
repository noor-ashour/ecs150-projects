#include <iostream>
#include <string>
#include <vector>
#include <assert.h>

#include "LocalFileSystem.h"
#include "ufs.h"
#include <string.h>

using namespace std;

LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}

void LocalFileSystem::readSuperBlock(super_t *super) {
  //Disk *disk = this->disk;
  disk->readBlock(0, super);
}

void LocalFileSystem::readInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
  //Disk *disk = this->disk;
  disk->readBlock(super->inode_bitmap_addr, inodeBitmap);
}

void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap) {

}

void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap) {
  //Disk *disk = this->disk;
  disk->readBlock(super->data_bitmap_addr, dataBitmap);
}

void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap) {

}

void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes) {
  //Disk *disk = this->disk;
  disk->readBlock(super->inode_region_addr, inodes);
}

void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes) {

}

int LocalFileSystem::lookup(int parentInodeNumber, string name) {
  inode_t parentInode;

  // Read the parent inode given its number
  if (stat(parentInodeNumber, &parentInode) != 0) {
    return -EINVALIDINODE;
  }

  // Check parentInodeNumber is a parent
  if (parentInode.type != UFS_DIRECTORY) {
    return -EINVALIDINODE;
  }

  for (int i = 0; i < sizeof(parentInode.direct); i++) {
    int blockNum = parentInode.direct[i];

    // Check if this is an empty directory
    if (blockNum == 0) {
      break;
    }

    size_t dirEntriesSize = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
    dir_ent_t *dirEntries = new dir_ent_t[dirEntriesSize];
    disk->readBlock(blockNum, dirEntries);

    // Search through the current directory entry block for name
    for (int j = 0; j < dirEntriesSize; j++) {
      if (strcmp(dirEntries[j].name, name.c_str()) == 0) {
        delete[] dirEntries;
        return dirEntries[j].inum;
      }
    }

    delete[] dirEntries;
  }

  // If we reach this point, file was not found
  return -ENOTFOUND;
}

int LocalFileSystem::stat(int inodeNumber, inode_t *inode) {
  super_t superBlock;
  readSuperBlock(&superBlock);

  int inodes_per_block = UFS_BLOCK_SIZE / sizeof(inode_t);

  inode_t *inodes = new inode_t[inodes_per_block];
  readInodeRegion(&superBlock, inodes);
  
  int offset = inodeNumber % inodes_per_block;

  inode_t *target_inode = &inodes[offset];

  inode->type = target_inode->type;
  inode->size = target_inode->size;

  for (int i = 0; i < sizeof(target_inode->direct); i++) {
    inode->direct[i] = target_inode->direct[i];
  }

  // TODO handle errors

  delete[] inodes;
  return 0;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size) {
  super_t superBlock;
  readSuperBlock(&superBlock);

  //inode_t inodeTable[num_inodes];
  //inode_t inode = inodeTable[inodeNumber];
  
  int pos = 0;
  //int index = pos / UFS_BLOCK_SIZE;
  int offset = pos % UFS_BLOCK_SIZE;
  //int blockNum = inode.direct[index];
  
  char block[UFS_BLOCK_SIZE];
	// TODO readDataBitmap(blockNum, block);
	memcpy(buffer, block + offset, size);

  // TODO on success, return number of bytes read
  return 0;
}

int LocalFileSystem::create(int parentInodeNumber, int type, string name) {
  return 0;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
  return 0;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name) {
  return 0;
}

