# Remote File Systems Research Questions to Ask
1. Are MAC addresses unique?
   1. How it relates to ARP
   2. How it works when we are not in a local network
2. Where in the code does *netin* call ip_enqueue?
   1. In *arp_in*, if a reply is needed it just writes to the ethernet device directly without queueing
   2. In *udp_send*, also just writes directly
3. If only one write request for a given block on the request queue can be done at a given time, how does it work if process 1 does a write op, process 2 does a read op, then process 1 does another write op?
   1. If the first write op gets overwritten by the second one, won't process 2 not read the first write?
4. Explain how caching works in the remote disk?
   1. What does the reference count represent?
5. Differences between remote disk and local disk systems?
   1. Conceptually, are they the same just that remote disk makes networking calls while local disk would just make calls to disk driver?
6. Will I have to interact with the disk drivers at all?
7. It appears that *read* and *write* to disk are slightly different than normal read and write?
   1. For example, the line "read(diskdev, dbuff, ib2sect(inum));" seems to be reading into dbuff the contents of diskdev at location ib2sect(inum), which is different from the normal read which would put in ib2sect(inum) bytes into dbuff from the current file pointer location of diskdev.
8. So only one i-block and d-block for a given file are loaded into memory at any given time?
9. It is stated that Unix treats files as devices where in Unix we treat devices as files? How does this affect implementation, and what's the reasoning for these choices?
10. So for a remote file system, we don't get a file descriptor for a specific remote file? We just send messages to the server directly, and in our request specify the file name?
    - Answer: there are pseudo-devices for remote files, but when performing file operations on them, they are translated to sending messages "under the hood"
11. How does a generic *open* call on the RFILESYS end up invoking the correct *rfsopen* function?
    1.  In general, how do the *open* calls get mapped to the correct internal function?
12. What is the rfs mutex used for?
13. How do directories work in the rfs?
14. Does remote file system need to be a specific entry in the device table?