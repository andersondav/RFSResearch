# Research Log
## Remote File Systems Research
## Anderson David under Prof. Douglas Comer
------
## 1/18/21
### Notes
- Downloaded/extracted version of Xinu and version of new server
- Wrote code for user to input remote server's IP address
- Started testing connection to remote file system

### Issues/Improvements 1/18/2021
1. ![](+.png) Problem with initializing network setup
   - Solution: had to uncomment out call to *net_init* in system/initialize.c
2. ![](+.png) Problem with receiving TIMEOUT on open request
   - Solution: [See 1/20 Issue #1]

## 1/20/21
### Notes
- Met with Pr. Comer to discuss some questions
- Went over problem encountered on 1/8

### Issues/Improvements 1/20/2021
1. ![](+.png) [1/18 Issue #2](#issuesimprovements-1182021)
   - Solution: for some reason, the Xinu client was not retrieving its local IP address (when monitoring packets using *pdumph()*, the src was 0.0.0.0 rather than true IP). A call in *main* to *getlocalip()* fixed this. 
   - [See 1/25 Issue #1] for more permanent fix.

## 1/21/21
### Notes
- Tested reading/writing messages with new-rfserver
  - able to write a message, seek back to beginning, then read the message written to the file
  - data persists across client lifetime--if I restart the client I can still access info written by a previous client

### Issues/Improvements 1/21/2021
1. ![](+.png) Potential problem with reading more bytes than file length
   - Receives strange characters instead of just EOF; probably need to modify on the server side.
   - [See 1/25 Issue #2] for solution.
2. ![](+.png) Potential problem with opening then immediately reading a file on server start
   - Server seems to freeze, and all client requests from that point on TIMEOUT (even after restarting client and trying to open or write)
   - [See 1/27 Issue #1] for solution

## 1/25/21
### Notes
- Solved the issues stated below, and added a potential improvement, which I implemented but need to check with Prof. Comer if this is a useful design strategy.
  
### Issues/Improvements 1/25/2021
1. ![](+.png) [1/20 Issue #1](#issuesimprovements-1202021)
   - Problem with unretrieved local IP permanently fixed by adding a call to getlocalip() in net/net.c:70 in function net_init()
2. ![](+.png) [1/21 Issue #1](#issuesimprovements-1212021)
   - Reason for strange characters was that the buffer used in main() was not cleared after retrieving from call to getmem(). A call to memset() on the buffer to intialize it to all zeroes fixes the strange characters. Server sends correct response length.
3. ![](`.png) Potential improvement to rfsread()
   - A potential improvement to rfsread() would be to only copy min(count, resp.rf_len) bytes into the buffer. If resp.rf_len > sizeof(buffer), then rfsread() would overwrite potentially sensitive data.

### Questions
- Whenever the Xinu backend gets loaded, does it retrieve a new IP address from DHCP every time? If so, how does it reliably get the same address every time? 
- So sending udp messages, such as those required for the rfs communications, don't utilize the ipout process? What is that process used for then?

## 1/27/21
### Notes
- Reading through server code to fix 1/21 Issue #2
  - found that when I define a preprocessor constant DEBUG in rfserver.h, open followed by read works fine (regardless of what DEBUG is set to), but when it is not defined, server freezes
  - Based on debug messages seen on correct behavior, the server appears to send a response to the open request and then freezes after recv()ing the read request when it errors
  - Appears that, with DEBUG undefined, the open request does not actually place the file pointer into the open file table, as on the subsequent read request, we do not find the file in the open file table.
  - Actually, it seems that the opened file is placed in the open file table after the server handles the read request...
  - This is not due to messages arriving too quickly, as I tried sleeping for 5s between sending the open and read requests

### Issues/Improvements 1/27/2021
1. ![](+.png) [1/21 Issue #2](#issuesimprovements-1212021)
   - Determined that the problem is that, when the server receives an open request, it either does not actually open the file, or that the opened file is not placed in the open file table.
   - Turned out that the call to rsofile() for files that already exist in the server was grouped into the ifdef for DEBUG, so that the call to open a file that had been created before would not go through unless DEBUG was defined. Once I moved that out, everything worked fine.
2. ![](`.png) [1/25 Issue #3](#issuesimprovements-1252021)
   - Upon further inspection of how the server handles read requests, it actually just calls the Linux syscall read, and the number of bytes read by Linux is what is placed in the response's length. So, this mitigates the risk of buffer overflow, although it could still be useful on the Xinu side if we request to read more bytes than our buffer can handle, although this might be a responsibility that should be delegated to the programmer not the RFS.

### Questions
- In the server code, why do we reset the server's local sequence to 0 if the current packet's sequence is 1?
  - In general, what is the sequence number used for?
- How do you send a mkdir request in Xinu?
- Why, when reading a directory, does the server just return the next entry in the directory? Why doesn't it read all of the files in the directory?
  - Answer: this is how it's implemented in Linux. We use the readdir() function to get the next entry pointed to by our directory pointer. We can do the same thing in Xinu by just calling read() on our directory to get the next entry at each stage.

## 2/1/2021
### Notes
- Sent update email to Professor Comer
- Continued testing rfs functionality
  - In particular, testing retreieving file size and truncating a file
- Began investigation caching
  - The max size of data that can be returned from a single request is 1024 bytes, so that may be an ideal size of caching buffers

### Issues/Improvements 2/1/2021
1. ![](+.png) Potential improvement to rfscontrol()
   - As of now, when using rfscontrol to retrieve the size of a file, it does not seem to allow for the passing of a file name, and will only return the size of the first opened file (even if that file has been closed).
   - See [2/4 Issue #1](#issuesimprovements-242021) for solution


### Questions
- From the client perspective, can there be only one open file at a time? And if not and we can have multiple, how does the client retrieve the size (or any control function) of specific files, because file size requests are sent to the RFILESYS device itself, not specific open file objects?

## 2/3/2021
### Notes
- Continued investigating how rfscontrol() works with file size requests, specifically how it decides which file to request size of
  - Tried to call rfscontrol on the remote file directly rather than the control block, but this just led to an error
  - Interestingly enough, when you close all the files and then open the one you want size of, then when you request file size it is correct. So, I think since the RFILESYS only has 1 control block, its minor number is 0, so in rfscontrol we mistakenly try to access the 0th control block of the remote file control block array, which will always be the remote file occupying the first entry in this array.
- Found that when closing a remote file, we don't actually inform the server of the file being closed, we just free the device table entry for the remote file

### Issues/Improvements 2/3/2021
1. ![](+.png) [2/1 Issue #1](#issuesimprovements-212021)
   - After some more investigating, confirmed that there is no way to pass a specific file into rfscontrol() when requesting file size.
   - I believe the problem is due to the fact that rfscontrol() gets called on the RFILESYS pseudo-device rather than a specific remote file pseudo-device.
   - In rfscontrol(), we obtain a pointer to the control block of the remote file we want to get information of, but this is incorrectly computed because we use the RFILESYS's device number instead of the remote file's pseudodevice
   - See [2/4 Issue #1](#issuesimprovements-242021) for solution
2. ![](+.png) [1/27 Issue #2](#issuesimprovements-1272021)
   - After talking with Prof. Comer about this, realized that since the request size gets it number of bytes from size of user's buffer, there should be no way that the buffer overflows, unless the server returns more bytes than the requested data, which I may implement for caching purposes. Otherwise, this should not be an issue presently.

## 2/4/2021
### Notes
- Working on fixing problem with rfscontrol when obtaining file size
  - Right now, I see two possible ways to fix the problem: 
    - First, we could just make it so that when you call control() on RFILESYS with RFS_CTL_SIZE, rfscontrol() will read the "arg1" argument passed in as a file name to retrieve size of.
    - Second, it seems that the intended usage (and what would make more sense possibly) is that you can call control() on the remote file device directly to obtain the size of that file. So, I could change the config so that calling control on a rfl pseudodevice will redirect to rfscontrol(), and then in rfscontrol(), I can see if the calling device is a remote file pseudodevice I'll only allow RFS_CTL_SIZE to go through, otherwise I'll call ioerror() as the current implementation does.
  - The main pro with the first potential fix is that I can get the file size of any file I want, even ones that aren't open currently.
  - The pro of the second potential fix is that it seems more intuitive from a programmer perspective, in that if you have an open file descriptor you should be able to get the file size by calling control() on that object itself.
  - Maybe it would be worth it to implement both versions, and just add some initial logic to rfscontrol() that can make it function differently if called from RFILESYS or on a specific remote file.
  - Found that the function rfsndmsg() actually implements the first approach already, by substituting a specified file name in place. However, this function is used for messages whose reponse is only success or error, such as making a directory or truncating/deleting a file. Therefore, probably cannot use it for file size request without alteration.

### Issues/Improvements 2/4/2021
1. ![](`.png) [2/3/2021 Issue #1](#issuesimprovements-232021)
   - Implemented fix #1 where, in the case of a file size request, substituted the passed in file name as the message's file name. This seems to fix the problems.
   - Also implemented fix #2, so you can call for a file size request on a remote file object directly by calling control() on the descriptor. However, if you try to call any control func other than RFS_CTL_SIZE, you get an error.

### Questions
- What's the best way to handle the above issue?

## 2/8/2021
### Notes
- Changed working directory to a fresh copy of xinu from xinu website instead of version used in CS354.
  - Found that the problems encountered early on with getting local ip were not present when using a fresh version.
  - My version of rfscontrol() seems to work with this version as well, as I can still communicate with the rfserver and get file size by either specifying file to RFILESYS device or calling it on rfl device directly.
- In regard to the potential buffer overflow issue presented in [2/3 Issue #2](#issuesimprovements-232021) and prior, received guidance from Professor Comer that, although this would not be a problem if the client-server interactions go through as expected, it could be a vulnerability if attackers try to compromise a Xinu system by sending spoofed responses that would overflow the buffer. So, I guess it is safer to keep this improvement. 
- Instead of writing all testing code in main(), I added a file called rfstesting.c, and in it have various functions to specify each test that I conduct. That should clean up the code in main() and make it easier to keep track of what I am testing at any given time. 

### Issues/Improvements 2/8/2021
1. ![](`.png) Potential Improvement for Size Requests
   - Noticed that when we request the size of a file that does not exist, server responds with 0. Would it be better to respond with SYSERR?

## 2/10/2021
### Notes
- Testing creating/removing directories
  - Creating a directory works fine
  - For removing directories, I noticed that if the directory is empty, I can remove it if it is empty, but if it is not empty, I cannot. I looked up how it is implemented in Linux, and directories must be empty for rmdir() syscall to work there, so it is probably good to keep it like this. Maybe add functionality to recursively delete directories down the line.

### Issues/Improvements 2/10/2021
1. ![](+.png) [2/4 Issue #1](#issuesimprovements-242021)
   - For fix #2 where we can call control() on a rfl device, there is a problem where if we try to call it on a closed device pointer, that the size returned is still equal to that of the last-open device pointer in that slot.
   - Fixed issue by returning SYSERR if the device is an rfl device and its state is RF_FREE
2. ![](`.png) Potential Improvement for Creating Directories
   - In the current implementation, if we try to create a directory that already exists, then the server just returns SYSERR. Would it be useful to respond with a more specific error message to let the user know that the request failed because the directory already exists to differentiate from it failing for some other reason?
3. ![](+.png) Improvement to Reading Directories
   - Because we do not close files on the server side when calling close in Xinu, the directory pointer on the rfserver is never closed. So, if we open a directory on Xinu and read through it, we will as expected get the contents of this directory. However, if we close then re-open this directory on the Xinu side and try to read through again, we will get no items back. This is because the directory pointer was never closed on the server side, so it still points to a NULL entry since from its perspective the directory has been read-through, while from the Xinu side we are using a new device pointer entirely.
     - See [[2/12 Issue #2](#issuesimprovements-2122021)] for solution

## 2/12/2021
### Notes
- Working on fixing [2/10 Issue #3](#issuesimprovements-2102021)
  - One potential fix is that whenever we send an open request to the server and the server decides that directory is already open, we just close and re-open that directory.
  - Alternative fix: use seekdir() to seek to the beginning of the directory every time it is opened

### Issues/Improvements 2/12/2021
1. ![](+.png) Implication of [2/10 Issue #3](#issuesimprovements-2102021)
   - It appears that this problem applies to regular files as well--if we open, read, close, then open again from the Xinu side, won't the server have the file pointer position set to however many bytes were read? We should probably reset the file position on every open, file or directory.
     - Actually, for rsread, we specify the offset to read from on every read request, so even if the pointer was last set somewhere else, whenever we open the file on the Xinu side, from its perspective the position is at 0 and it will send this position to the server on every read.
   - This also begs the question of what if we have two file pointers to the same file, as in this implementation they would basically be sharing a common file pointer...
     - Well, technically this works because from the Xinu side every file pointer will have its own position, and we can just use the single file pointer on the server side to jump back and forth to wherever these pointers may be
- ![](+.png) [2/10 Issue #3](#issuesimprovements-2102021)
  - Fixed the issue by adding some logic that, when the file is already open on the server and is a directory, that we will seek to the beginning of the directory using seekdir().
  - Potentially, could implement a similar logic to normal files where we specify a position to read from every time... actually this is already implemented

## 2/15/2021
### Notes
- Going to start formulating how to implement local caching
- Ideas
  - Every read request will request a pre-set number of bytes from the server. Let's call this *rfl_chunk_size*
  - Xinu will dynamically allocate memory to store *rfl_chunk_size* bytes returned from a read request, as well as indicating what file they came from and what bytes in the file they correspond to (ex: bytes 0 through *rfs_chunk_size*, bytes 24 through *rfl_chunk_size* + 24, etc.). Also will keep track of validity of chunks.
  - These chunks will be stored in **some data structure** called *rfl_chunks*
  - Reading
    - Whenever we make a read request, we consult *rfl_chunks* to see if the bytes requested are already in the cache, and if they are valid. If so, just read from *rfl_chunks*, and if not make a request to server.
  - Writing
    - Upon some inspection, writing is implemented like so: if we write past file length, we will start appending. Otherwise, when we write to bytes that already have information, we will overwrite existing data (this makes caching implementation a little easier)
    - From this, we have a few options
      - Option 1) writes invalidate the cache: whenever we overwrite file data, that section of the cache becomes invalidated.
      - Option 2) writes get applied to the cache: whenever we overwrite file data, we update that chunk of the cache. We can introduce a dirty bit to this that indicates blocks whose data have been changed. Then, when we decide to propagate changes to server, we can send write requests for dirty blocks. This can occur as writes occur, or when we close the file.
  - Deletion/Truncation
    - Just set all chunks as invalid
    - Alternatively (or additionally), de-allocate these chunks
  - Closing the file
    - Same as deletion/truncation
  - Size requests
    - keep track of file size in cache
  - Directories
    - TBD...
- Things to consider
  - What is the best way to store the cached chunks?
    - The most straightforward way is probably with an array, and if we order the chunks by their corresponding file location, then we can access a chunk in constant time.
    - This gets more complicated when we have to start considering eviction though
    - Alternatively, we could use a tree-like structure based on starting file location, but this could require some overhead, and again evicting would be troublesome as we would have to delete from the tree
  - Constants
    - *MAX_CHUNKS*: the maximum number of chunks a remote file can have in its cache --> TBD...
    - *RFL_CHUNK_SIZE*: the number of bytes a cache chunk stores --> Right now, the RFS limits data length to 1024 (RF_DATALEN) bytes, which seems reasonable
  - Dealing with partially populated chunks
    - Inevitably, there will be some chunks where only part of the data is populated (Ex: If chunk size is 16, a file of size 24 will have one full chunk and one half-filled chunk)
    - Could set the un-used portions of the chunk to null chars, and could add a size field to ensure returned bytes are in the boundaries of this size
    - Alternatively, each chunk could keep track of the number of bytes in its data that are valid bytes (the sum of these across all chunks should equal the size of the file)

### Issues/Improvements 2/15/2021
1. ![](`.png) Potential Improvement for Deleting Files
   - When making a delete request, on the server side, we close the file if it is open, and then call unlink() to delete the file. Would it be better to do the same on the Xinu side, because right now if we open a file then delete it, the Xinu process that opened that file has no idea it was deleted.

### Questions
- Questions on things to consider in notes
  - Eviction and size of cache: How to determine what is a good cache size?
  - Writing: Is it better to write to server on every write or when closing the file, or some other policy?

### Images
![](images/IMG_1740.jpg)

## 2/17/2021
### Notes
- Beginning implementation of caching
  - Choosing constants
    - MAX_CHUNKS: the maximum number of cache chunks a remote file can have in its cache at any given time: 10
    - RFL_CHUNK_SIZE: the size of each cache chunk, which by consequence will be the default size of every read operation: 1024, equal to RF_DATALEN
    - This means that struct rfl_chunk will require (32+8+1024+32)/8 = 137 bytes, and if we have 10 chunks requires 1370 bytes, and if we have 10 rfl devices that is 13700 bytes at maximum needed for caching.
    - 13.7 KB does not seem too bad, but we can tweak these numbers if need-be
- How will we handle a read request that is bigger than, or requires use of, multiple cache chunks?
  - If we set RFL_CHUNK_SIZE to RF_DATALEN, then at worst we might need info from 2 different cache chunks...

### Issues/Improvements 2/17/2021
1. ![](-.png) Starting to implement caching
   - Files added:
     1. include/rfs_cache.h: Contains constants and data structure definitions relevant to caching.
     2. device/rfs/rfscache.c: Contains useful functions for caching.
   - Functions added:
     1. device/rfs/rfscache.c/rfs_check_cache(struct rflcblk rfptr, char *buff, int32 count): Copies over as many bytes as possible from cache into buffer
     2. device/rfs/rfscache.c/byte_is_valid(struct rfl_chunk *cur_chunk, uint32 byte_number): determines if a given byte_number is valid within a given cache chunk
   - Based off this implementation, we assume that cached chunks are kept in order
     - So, if we go to chunk 9 to find bytes 0 through 10 of the data, then we assume bytes 11 onward won't be in previous chunks...

### Questions
- In the Xinu code, for many operations such as the local disk system, we pre-allocate memory regions as a buffer pool for potential buffers. Why exactly is this desirable to using getmem(), and should we use this concept for rfs caching as well?

## 2/22/2021
### Notes
- Meeting with Professor Comer
- Updates to Code
  1. can now pass a filename into rfscontrol() for obtaining file size, and can also call it on the rfl device directly (only if registered and a size request)
  2. For directories, whenever we re-open a directory on the Xinu side, the server will seek that directory to the first entry, so that reads can happen on the newly opened directory from the Xinu side without returning NULL every time
  3. Began implementing caching
     - Added a cache field to all rfl devices, which is an array of 10 cache chunks
     - Each cache chunk can hold 1024 bytes of data to match the max-size data that rfs can request
     - Each chunk has a field for the starting byte position, and also a field for the number of valid bytes, because if file is 500 bytes for example, only first 500 bytes of the chunk have valid info
     - Also have a dirty bit for writing
     - Started modifying rflread(), so before making a request we look in that rfl device's cache for data and return it if found.

### Questions
- For caching, how to decide a good size of cached blocks and the number of blocks we can have in memory simultaneously?
  - depends on use case, so probably not all files should be limited to same number of blocks
  - we can allocate blocks as needed, and de-allocate based on least recently used system (or some other policy) across all data blocks for all remote files, not just for each remote file individually
- Should we dynamically allocate or have pre-allocated cache block buffers, and why?
  - dynamically allocated cache blocks allow user to have some control over how we allocate memory, so a malicious or ignorant programmer could cause us to allocate a lot more memory than we should
  - having pre-allocated cache blocks allows us to determine ahead of time how much memory we have at our disposal
- Should cache blocks be kept in order? And if so, is the overhead of doing so worth it?
  - probably a good idea, but also depends on use case
- Is an array structure fine for caching, or should we use a linked list like we do with i-blocks?
  - for small files, it might be easier to have an array of blocks for each portion of the file (ex: arr[0] -> bytes 0-1023, arr[1] -> bytes 1024-2047, etc.). This way, whenever we want to access a specific byte-region, we can just do some math to reach it. This is better than having to search through a linked list of blocks.
  - for large files though, if we keep this system, then our array will be quite large. So then in that case it might be better to have a linked list, but we do incur overhead for having to search through it/keep it sorted.
  - a hybrid solution uses an array for the first x block-sizes of the file, then a linked list for any remaining blocks. With this we get the ease of array operation for the first x block-sizes (which is sufficient for most files), and then a space-saving linked list approach for larger files (of which there won't be many, so the expensive search will not happen often).
- In general, should I mirror the cache to an i-block/d-block style?
  - Yeah kinda
- For writing, should we send update requests to the server on every write, or just whenever we close the file, or at some other frequency?
  - Option 1: write through--whenever we write to that cache block, we also write back to the server through a write request
  - Option 2: write back--whenever we write to that cache block, we don't write to the server. At some later point, such as when the file is closed or that block is evicted, do we actually write the changes to the server.
- What if the byte-range we are writing to isn't in the cache yet?
  - Option 1: we can read in that portion of that file into the cache, and then write the changes into the cache to avoid the problem
    - this could be inefficient if we are bringing in 1024-byte blocks just to write 4 bytes and then never use that block again
  - Option 2: we use some other bookkeeping to keep track of writes, such as a bitfield that corresponds to each byte in the block that has been altered, so we save space by avoiding reading that block into memory
    - however, if we accumulate lots of writes to a block, then this also gets very tedious to send all those write requests to the server
- For determining if a byte is valid...
  - Each block could contain info about number of valid bytes (so the last block would have less than full block-size)
  - Each read request returns a file size additionally, and each time we read a byte from that block we check if overall file size is exceeded

## 2/23/2021
### Notes
- Continuing to implement caching
- Revising according to Prof. Comer's suggestions
  1. Change "cache chunk" to "cache block" for more correct terminology
  2. Rather than just an array of the 10 cache blocks in memory, I will keep space for many cache blocks, but will only allocate space for ones currently in memory. This allows me to quickly lookup what I need.
- Found that it will probably be very useful to truncate reads at the beginning so they won't go beyond size of file.
- Might also be useful to divide read into multiple reads that each will access at most one block 

### Issues/Improvements 2/23/2021
1. ![](+.png) Important case to consider: required data lies in multiple cache blocks
   - ex: user wants to read 1024 bytes starting from position 512, which requires 512 bytes from block 1 and 512 bytes from block 2
   - Proposed solution: we will split this into two read requests of size 512 each that fill in different parts of the buffer
   - Future problem to consider: what if user wants to read more than 1024 bytes (may require > 2 blocks)
     - Could have cascading recursion which breaks into multiple blocks at a time
   - Perhaps more problems if block 2 is in cache already, but block 1 is not
   - Even more difficult is what if file is only 1024 bytes long?
     - May be useful in this case to retrieve file size every time, and truncate read request to available bytes before looking in cache
    - addressed in [3/13](#issuesimprovements-3132021)

### Questions
- Is it a bad idea to use recursive calls in systems programming, due to the potential of running out of stack space?

## 2/24/2021
### Notes
- Due to yesterday's issues that creeped up, I am going to change what the server returns for read requests
  - Every read response will also contain file's current size
- Also, when rflread is called, I will split the request into potentially multiple smaller requests so that each of these requests will only rely on data from one cache block 

### Issues/Improvements 2/24/2021
1. ![](-.png) Problem with concurrent access
  - If I try to read from Xinu side, then edit file in vim, then read again, changes are not reflected
2. ![](`.png) Potential Improvement for getting file size
  - Need to see if there is a reliable way to update the file size when making a size request via control(RFILESYS, ...)
  - Only way I can think of now is to loop through all open file devices to find a matching filename, and update that file device, but could be prone to error
3. ![](+.png) Read requests now return file size as well, and there is a field in each rfl's control block for current file size.

## 3/1/2021
### Notes
- Will continue working on caching
  - From [2/23 Issue #1](#issuesimprovements-2232021), want to be able to truncate requests that go beyond file size, and split requests that span multiple cache blocks.
  - For now, will still limit read()s to 1024 bytes, so worst case is that it will span two cache blocks
  - Need to fix rfs_check_cache() now that I'm doing this pre-processing inside rflread() itself.
- Things to consider
  - Remember: with this new implementation, all network requests will be for one full cache block, so its size must be RF_DATALEN and its position must be a multiple of RF_DATALEN.

### Issues/Improvements 3/1/2021
1. ![](+.png) A potential side effect to caching is that since we will cache any bytes returned by a network request and all coyping into user buffer comes from the cache, we can properly vet data returned by the network before going into cache rather than copying it directly into the user buffer. This is potentially useful as it pertains to [2/3 Issue #2](#issuesimprovements-232021)
2. ![](+.png) Progress on caching
    - rflread() now truncates read() requests so that they do not go beyond the most recently requested file size
    - rflread() also splits read() requests into 2 sub-requests if the request requires info from 2 cache blocks instead of 1
    - After creating the sub-requests, rflread() then loops through each of them, searching for the info in the cache. If not found, a network request for the needed cache block is made.
    - (TODO) After this network request is made, the returned data should be put into the cache, then we try fetching from cache again.
    - Also redid some index/offset calculations with bitwise operators instead of semi-complicated arithmetic
    - finished, see [3/13](#issuesimprovements-3132021)

## 3/3/2021
### Notes

### Issues/Improvements 3/3/2021
1. ![](`.png) Progress on caching
    - Thing to consider: when we make our first read request, we do not know the file size yet... potential solution could be to make a file size request to populate this field.
    - Successfully implemented storing a block in the cache
    - Tested that a basic use case of rflread() works with caching!
    - To do:
      - revise rfs_cache_store() to utilize buffered memory instead of getmem()
      - add code to handle what happens when we need to utilize the rfl_cache_list (Implemented and tested in [3/17](#issuesimprovements-3172021))
      - more testing/debugging, particularly for requests that don't use index 0 and/or offset 0 (Implemented and tested partially in [3/17](#issuesimprovements-3172021))

### Questions
- In rflread(), why do we wait() and signal() the Rf_data mutex? We do not interact with the Rf_data at all until we make the rfscomm() call, so why not just put the wait() and signal() calls in there to reduce the size of the critical section?

## 3/8/2021
### Notes
- Continuing to test caching implementation
  - Basic reading: works well
  - Reading from a partially filled block: works

### Issues/Improvements 3/8/2021
1. ![](+.png) Progress on caching
    - Caching works with basic read operations. We can correctly return requested data when the data is not in the cache (need to send network request), or when it has already been stored in the cache.
    - Also fixed an error with rfs_cache_store where we didn't convert file pos in response to host order. Now the info is stored in the correct cache block.
    - Spent some time making helper functions to generate test messages so it is easier to debug and create tests
    - To-do: still need to test a read operation that utilizes multiple cache blocks (done: see [3/13](#issuesimprovements-3132021))

### Questions

## 3/11/2021
### Notes
- Continuing testing
  - Fulfilling a read that spans over two cache blocks
    - doesn't seem to error out, but the message returned is slightly different from expected output, not sure why

### Issues/Improvements 3/11/2021
1. ![](+.png) Need to figure out why message output is slightly different from expected output.
    - maybe cache_store copies a byte twice...
    - or in cache_fetch our copying of bytes into buffer is off by one byte
    - fixed in [3/13 Issue #1](#issuesimprovements-3132021)

## 3/13/2021
### Notes
- Continued testing
  - Fixed problem from 3/11, can now read from cached data spanning multiple blocks without server even being on.
- Implementing cache_list
  - For the future when we need to evict blocks, it will probably be useful if this is a double linked list, so I will update the definition.
  - I think (for now at least) I will insert blocks into the list in sorted order, so that we don't have to iterate over all the blocks every time we fetch from the list.
    - In the future, it might be worth placeing in a binary tree

### Issues/Improvements 3/13/2021
1. ![](+.png) [3/11 Issue #1](#issuesimprovements-3112021)
    - problem was that the count on the first request was off by one, so read from first block read one block too few and so the read from second block read one block too much
2. ![](+.png) Cache List Implementation
    - working on implementing the cache list, have a basic implementation that seems to work on a very basic test
    - still need to test multi-block reads from list
    - Finished in [3/15](#issuesimprovements-3152021) and [3/17](#issuesimprovements-3172021)

## 3/15/2021

### Issues/Improvements 3/15/2021
1. ![](+.png) Testing cache list ([3/13 Issue #2](#issuesimprovements-3132021))
    - Testing basic read from cache list...done
    - Testing that sorted insertion works correctly...done
    - Finished in [3/17](#issuesimprovements-3172021)

## 3/17/2021
### Notes
- Finishing up tests for the cache list
- Investigating how the buffer allocation works
  - we can use mkbufpool(bufsize, numbufs) to allocate a set number of cache blocks beforehand. It returns the poolid of the buffers, which we'll need to save somewhere (probably in Rf_data)
  - then we can use getbuf(poolid) to retrieve one cache block at a time, and freebuf(bufaddr) to return the block to the buffer pool
- How should we use buffer allocation?
  - Allocating is easy--just replace the calls to getmem() with getbuf() and in rfsinit() we can use mkbufpool() to initialize the buffer pool
  - De-allocating is the hard part: when to de-allocate a buffer pool
    - Option 1: whenever a file is closed, go through all its cache array and cache list and de-allocate all allocated buffers
      - could take a while if the file had a lot of cached data
    - Option 2: LRU
      - maintain a list of most recently used cache blocks, will probably also be stored in Rf_data
      - the head of the list will contain the most recently used cache block, and the tail will contain the least recently used cache block
      - We will keep a count (in Rf_data) of how many blocks have been allocated so far. Once that count reaches the number of buffers originally pre-allocated, we will use the LRU list to determine which block to evict and replace.
      - With this, we don't really need to manually evict all blocks of a closed file, as those will naturally become less recently used since that file is closed.
- How to implement LRU list
  - since all the remote files are sharing buffers, they should also share the LRU list, so it should reside in Rf_data
  - to easily remove items from the list, we should make the list doubly linked
  - we should store pointers to next and prev in the list in the block itself, so the block can easily update these whenever it is fetched
    - for consistency, might be worth it to place the next and prev for the rfl_cache_list also in the block's definition itself, and just set both to NULL if using the array portion
  - Rf_data should keep the current head and tail of the list, because new items get added at the head and items get evicted from the tail end
  - As mentioned above, Rf_data should keep count of the number of allocated blocks, which is also the number of blocks in the LRU list
  - Whenever a block is fetched, it gets put at the head of the list
    - If the block is just now being allocated for the first time, it will get added at the head of the list immediately.
    - If the block had already existed before the current fetch, it will be removed from its current spot in the list, and then re-inserted at the head of the list.
    - Can implement this by rfs_cache_store() just allocating the buffer and setting its next and prev to NULL, and rfs_cache_fetch() handling all LRU list operations. If the block's next and prev are both NULL, we know it hasn't been added to the list yet, so just insert it at the front. If either of these are not NULL, we know it's in the list somewhere currently, so just remove it then re-add.
  - When rfs_cache_store() tries to allocate but sees that the limit has been reached, we'll evict a block.
    - We'll take the block at the tail of the list as the block to evict.
    - We don't actually need to de-allocate it, as it will just be re-allocated again immediately since we are evicting because a new block neeeds to be allocated.
    - If it's in the array portion of the cache, we can use the file_start field to find the correct index and set that entry to NULL. We'll also need and extra field for each block to indicate which remote file they are associated with if we are going to go this route.
    - If it's in the list portion of the cache, we can just remove it.
    - After this, we just give this evicted block to rfs_cache_store() and it will populate the block with the new info, and then rfs_cache_fetch() will add it to the LRU list in the subsequent fetch.
- Something to think about: how will writes affect all of these mechanisms?

### Issues/Improvements 3/17/2021
1. ![](+.png) Testing cache list[3/13 Issue #2](#issuesimprovements-3132021)
    - Testing that read across multiple blocks in list works correctly...done
    - Testing that read across last block in arr and first block in list works correctly...done

### Questions
- How to decide what's a good number of blocks to buffer via array?
  - Too small may cause us to resort to linked list, which takes time to search through
  - Too large may take too much of a toll memory-wise
- What's a good number of cache blocks to pre-allocate with the buffer pool
  - Too small and we'll have to evict a lot and may not be able to cache an adequate number of blocks
  - To large and it takes a strain on the memory of the system

## 3/19/2021
### Notes
- Beginning to implement LRU list

### Issues/Improvements 3/18/2021
1. ![](-.png) Implementing the LRU list
    - Going to consolidate cache linked list into the rfs_cblock definition itself
    - Updated some variable names in code
    - New variable for rfs_cblock: devnum - gets initialized in rfs_cache_store
      - Verified that we can access a block through it's devnum
    - Need to start implementing lru list addition and eviction procedures

## 3/22/2021
### Notes
- Continuing to implement LRU list

### Issues/Improvements 3/22/2021
