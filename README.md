# RFSResearch
## Anderson David under Prof. Douglas Comer

## Description
This repo is the work of an independent study research done during the Spring 2021 semester. It aims to explore how a remote file system operates, including its use cases, advantages, disadvantages, and ways to improve their reliability and usefulness. In particular, this research studies the remote file system present in the Xinu operating system, and I, under Professor Comer's guidance, will spend the semester exploring how the remote file system is implemented in Xinu and will try to fix any bugs that I find, make any improvements to the existing functions, and try to expand the remote file system to handle additional use case. 

## Tasks Accomplished
1. Tested the various functions present in the out-of-the-box implementation of the RFS in Xinu.
2. Made improvements to existing functions
  - Fixed small bug in server code where some rfs open() calls would fail.
  - Added some protections against potential buffer overflow when reading on Xinu side by limitting number of bytes copied into user buffer to minimum of response size and user's buffer size.
  - Added functionality to rfs control() function to allow users to pass in filename when requesting file size, or to request file size from a remote file device directly.
  - Fixed bug where opening, reading, then re-opening a directory on the Xinu client side would not re-set directory pointer to beginning of directory.
 3. Caching in the Remote File System
  - Implemented a local file cache on the Xinu client. This system would enable the Xinu client to cache file data from past file accesses so that potential future accesses would not require network calls, thus improving efficiency.
  - Link to testing results of cache: https://docs.google.com/document/d/1y47y16yz3daPWNkbycYcKHH_TzWcvxRNoHLUZYM6E9c/edit?usp=sharing
  
## Technologies Used
1. The Remote File System is built into the Xinu operating system. The source code contains various RFS functions that are mounted onto the Xinu device table, thus allowing users to seamlessly communicate with remote files through conventional open(), close(), read(), write(), and control() syscalls.
2. The Xinu client communicates with a Remote File Server that is written in Linux (located in the 'new-rfserver' directory of this repo). This server responds to network requests from the Xinu client and creates/opens/deletes/reads/writes to RFS files via the default file system of the machine the server runs on.

## This Repo
This repo contains the source code I use to explore the Xinu file system. The top-level files and directories are described as follows:
  1. logs - Directory containing the file 'log.MD,' which outlines the day-to-day progress made on this research as well as supporting files for the Markdown document.
  2. new-rfserver - Directory containing the server code used for this research project. A compressed version of the default source code is in tarball new-rfserver.tar
  3. old-rfserver - Directory containing an old implementation of the RFS server [unused].
  4. xinu - Directory containing the Xinu implementation used. In particular, the directory xinu/fresh-xinu contains the working copy of the Xinu code currently being modified, while xinu/xinu contains a previous version that did not have all networking funcionality enabled. For the latest version of my work, please view files in xinu/fresh-xinu.
