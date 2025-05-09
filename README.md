# CSE109 - Systems Software - Spring 2025

# Final Exam

⏰ **Due by: 5/12/2025 EOD**

## Ethics Contract

**FIRST**: Please read the following carefully:

- I am the sole author of the content within this exam unless otherwise cited.
- I am not an uncredited author of any content in any other exam.
- I will not dicsuss the exam until after the submission deadline.
- All resources I used (including text books and online references, websites, language models), are cited in this exam.
- I will not plagiarize someone else's work and turn it in as my own. If I use someone else's work in this exam, I will cite that work. Failure to cite work I used is plagiarism.
- I understand that acts of academic dishonesty may be penalized to the full extent allowed by the [Lehigh University Code of Conduct][0], including receiving a failing grade for the course. I recognize that I am responsible for understanding the provisions of the Lehigh University Code of Conduct as they relate to this academic exercise.

If you agree with the above, type your full name next to the pen emoji, along with the date. Your exam **will not be graded** without this assent.

---------------------------------------------
🖋️Logan A Scheetz
---------------------------------------------

💥 **IMPORTANT:** When you are done, make your first commit with the commit message: `I, <your full name here>, agree to the ethics contract`.

💥 **IMPORTANT: As you are working on your midterm, commit your progress regularly.**


## Exam Prelude

This exam will test your ability to independently:

- Organize a project and write code in C or C++
- Implement a protocol for communication between a client and server
- Implement a program that communicates over Unix sockets
- Write unit tests to verify the correctness of your implementation

### Assignment

For this exam, you will write the server half of a client/server pair that will communicate over Unix sockets.

1. **File Server** - A file server is a program that hosts files for clients. It receives files that clients want to store, and sends them back to the client (or other clients) when they are requested. A compiled file server binary for several platforms has been provided for you in the `bin` directory. You can use it as a reference for this exam.

2. **Client** - This program connects to the file server and can send files to it, which are then stored by the file server. The client can also request files from the file server. A compiled client binary for several platforms has been provided for you in the `bin` directory as well.

### Completeness

As a general note / tip, this exam specifies the highest expectations for your exam submission. But in general it's better to submit an exam that answers all questions partially than an exam that only answer the fisrt few quesitons fully. 

You may find in the course of doing your exam, that cutting a corner or not meeting a requirement allows your life to be easier and allows you to get past something that's blocking you, you should take that shortcut. It might cost you a couple points in functionality/robustness, but you'll earn more points in completeness.

### Commit Policy

This exam is divided into discrete "questions", and you must make at least one commit per question. You can do the questions in any order, and you can go back to questions for which you have already made a commit, but you must make at least one commit per question. 

The commit message for each question should be "QUESTION N" where N is the number of the question.

### Resource Usage Policy

This repository is blank except for this README file. It's your job to create this program from scratch. You can use any resources from previous assignments to aid in your completion of this exam. Just be sure to cite any work you use from another source (even if you are the author). 

You are free to use any resource for this exam that are read-only. This includes books, notes, lecture videos, documentation, the internet, Stack Overflow, etc. But you are not permitted to request any help from resources, human or artificial intelligence or otherwise (unless permitted for a particular question).

You can use any libraries and functions you like to accomplish this except where stated otherwise.

If you use any resources from the internet or anywhere else, **YOU HAVE TO CITE THEM** in your exam. Failure to cite resources you used may lead to failure of the exam.

## Exam Overview

There are three parts to this exam:

- **Part 1** - Communication Protocol (30 points) - This part of the exam involves implementing a communication protocol between the client and server 
- **Part 2** - File Server Implementation (70 points) - This part of the exam involves implementing the file server that uses the communication protocol defined in Part 1
- **Part 3** - Video Explanation (Required) - This part of the exam involves recording a video explanation of your file server implementation and demonstrating that it works with the provided client. It's not graded, but you must complete it to receive credit for the exam.

### Part 1 - Communication Protocol

The server/client communication protocol consists of three types of messages:

- `File` - A message sent from the client to the server that contains a file to be stored by the server. This message is also sent by the server to the client in response to a successful `Request` message.
- `Request` - A message sent from the client to the server that requests a file stored by the server.
- `Status` - A message sent from the server to the client that indicates the status of a request. This message is sent when the server encounters an error while processing a request, or when the server successfully processes a request.

Each message is **serialized** using the Pack109 protocol, which we worked with in the previous assignment. You can use your solution to that assignment or the posted solution as a starting point. Remember to cite this code if you use it. 

The message is also **encrypted** using a simple XOR encryption scheme with a key of `42` before being sent over the socket. The client and server must both implement this encryption scheme.

A diagram of the protocol is shown below:

```
                                               +----------+
                                               |  Server  |
                                               +----------+
                           a. Server starts          |
                           b. Creates empty hash map |
                           c. Waits for client       |
+----------+                                         |
|  Client  |                                         |
+----------+                                         |
     |                                               |
     | 1. Create message                             |
     |    (File or Request)                          |
     | 2. Serialize                                  |
     | 3. Encrypt                                    |
     | 4. Send                                       |
     |                                               |
     |-->>>----------------->>>----------------->>>--|
     |                                               |
     |                     5. Receive                |
     |                     6. Decrypt                |
     |                     7. Deserialize            |  
     |                     8. Process message:       |
     |                                               |
     |                        If File                |
     |                         - Store file          |  
     |                         - Response: Status    |
     |                        If Request             | 
     |                         - Retrieve file       |
     |                         - Response: File      |
     |                        If error occurs        |
     |                         - Response: Status    |
     |                                               |
     |                     9. Create response        |
     |                        (File or Status)       |                          
     |                    10. Serialize              |
     |                    11. Encrypt                |
     |                    12. Send response          |
     |                                               |
     |--<<<-----------------<<<----------------<<<---| 
     |                                               |
     | 5. Receive                                    |
     | 6. Decrypt                                    |
     | 7. Deserialize                                |  
     | 8. Process response                           |
     |                                               |
```

### File Message

A `File` message is sent from the client to the server to store a file. The message contains the filename and the file data.

A `File` message is also sent by the client to response to a successful `Request` message.

If a file called `file.txt` with the contents `Hello` is serialzied, the *unencrypted* message looks like this:

```
┌───────────────────────────────────────────────────────────────────────────────┐
│0xae     // map tag                                                            │
│0x01     // 1 kv pair                                                          │
├───────────────────────────────────────────────────────┬───────────┬───────────┤
│0xaa     // string8 tag                                │           │           │
│0x04     // 4 characters                               │ key       │           │
│File     // the string "File"                          │           │           │
├───────────────────────────────────────────────────────┼───────────┤  pair 1   │
│0xae     // the value associated with the key is a map │           │           │
│0x02     // 2 kv pairs                                 │           │           │
├────────────────────────────────┬───────────┬──────────┤           │           │
│0xaa     // string8 tag         │           │          │           │           │
│0x04     // 4 characters        │ key       │          │           │           │
│name     // the string "name"   │           │          │           │           │
├────────────────────────────────┼───────────┤ pair 1   │           │           │
│0xaa     // string8 tag         │           │          │           │           │
│0x08     // 8 characters        │ value     │          │           │           │
│file.txt // the filename        │           │          │           │           │
├────────────────────────────────┼───────────┼──────────┤           │           │
│0xaa     // string8 tag         │           │          │ value     │           │
│0x05     // 5 characters        │ key       │          │           │           │
│bytes    // the string "bytes"  │           │          │           │           │
├────────────────────────────────┼───────────┤ pair 2   │           │           │
│0xac     // array8 tag          │ value     │          │           │           │
│0x05     // 5 elements          │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│H        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│e        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│l        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│l        // a byte              │           │          │           │           │
|0xa2     // unsigned byte tag   │           │          │           │           │
│o        // a byte              │           │          │           │           │
└────────────────────────────────┴───────────┴──────────┴───────────┴───────────┘           
```

In decimal:

```
[174, 1, 170, 4, 70, 105, 108, 101, 174, 2, 170, 4, 110, 97, 109, 101, 170, 8, 102, 105, 108, 101, 46, 116, 120, 116, 170, 5, 98, 121, 116, 101, 115, 172, 5, 162, 72, 162, 101, 162, 108, 162, 108, 162, 111]
```

In hex:

```
[0xAE, 0x01, 0xAA, 0x04, 0x46, 0x69, 0x6C, 0x65, 0xAE, 0x02, 0xAA, 0x04, 0x6E, 0x61, 0x6D, 0x65, 0xAA, 0x08, 0x66, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0xAA, 0x05, 0x62, 0x79, 0x74, 0x65, 0x73, 0xAC, 0x05, 0xA2, 0x48, 0xA2, 0x65, 0xA2, 0x6C, 0xA2, 0x6C, 0xA2, 0x6F]
```

### Request File Message

A `Request` message is sent from the client to the server to request a file stored by the server. The message contains the filename of the requested file.

If a requested file is called `file.txt`, then the *unencrypted* message looks like this:

```
┌───────────────────────────────────────────────────────────────────────────────┐
│0xae     // map tag                                                            │
│0x01     // 1 kv pair                                                          │
├───────────────────────────────────────────────────────┬───────────┬───────────┤
│0xaa     // string8 tag                                │           │           │
│0x07     // 7 characters                               │ key       │           │
│Request  // the string "Request"                       │           │           │
├───────────────────────────────────────────────────────┼───────────┤  pair 1   │
│0xae     // the value associated with the key is a map │           │           │
│0x01     // 1 kv pair                                  │           │           │
├──────────────────────────────┬───────────┬────────────┤           │           │
│0xaa     // string8 tag       │           │            │           │           │
│0x04     // 4 characters      │ key       │            │ value     │           │
│name     // the string "name" │           │            │           │           │
├──────────────────────────────┼───────────┤ pair 1     │           │           │
│0xaa     // string8 tag       │           │            │           │           │
│0x08     // 8 characters      │ value     │            │           │           │
│file.txt // the filename      │           │            │           │           │
└──────────────────────────────┴───────────┴────────────┴───────────┴───────────┘           
```

In decimal:
```
[174, 1, 170, 7, 82, 101, 113, 117, 101, 115, 116, 174, 1, 170, 4, 110, 97, 109, 101, 170, 8, 102, 105, 108, 101, 46, 116, 120, 116]
```

In hex:

```
[0xAE, 0x01, 0xAA, 0x07, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0xAE, 0x01, 0xAA, 0x04, 0x6E, 0x61, 0x6D, 0x65, 0xAA, 0x08, 0x66, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74]
```

### Status Message

When the server is done processing a `Request` or `File`, it sends a `Status` or `File` message back to the client to indicate the result of the request. 

- If the result of the request was successful, the `Status` message should indicate success. 
- If the result of the request was an error, the `Status` message should indicate the error and any information the client might need to diagnose the issue.

The serialized, *unencrypted* status message should look like this:

```
┌───────────────────────────────────────────────────────────────────────────────┐
│0xae     // map tag                                                            │
│0x01     // 1 kv pair                                                          │
├───────────────────────────────────────────────────────┬───────────┬───────────┤
│0xaa     // string8 tag                                │           │           │
│0x06     // 6 characters                               │ key       │           │
|Status   // the string "Status"                        │           │           │
├───────────────────────────────────────────────────────┼───────────┤  pair 1   │
│                                                       │           │           │
│                                                       │           │           │
│                                                       │           │           │
│                                                       │           │           │
│                      ???                              │ value     │           │
│                                                       │           │           │
│                                                       │           │           │
│                                                       │           │           │
│                                                       │           │           │
└───────────────────────────────────────────────────────┴───────────┴───────────┘ 
```

But this message is unfinished. It's up to you to decide how to handle errors and what the error message should look like. The important part here is to communicate to the client exactly what went wrong. Since you are designing this part of the protocol, the provided client does not implement it, so you won't be able to test it with the provided client binaries. However, you should still implement error handling in your server, and you can test it with your own client implementation.

**Success** can occur when the server processes a request and stores or retrieves a file. For example, when handling a `File` message, a status message can indicate:

 - successfully stores the file in the hash map and it didn't exist before
 - successfully replaces the file but it already existed, so the old one was overwritten

**Error** can occur in several places in your server implementation, including:

- When deserializing a message
    - malformed message
    - does not conform to the Pack109 protocol. 
    - message didn't decrypt correctly
- When inserting a file into the hash map
    - Not enough memory to store the file
- When retrieving a file from the hash map
    - File not found in the hash map

It's up to you how you want to handle these errors, but you should consider all possible error scenarios and how to ensure that your file server gracefully handles them, and communicates them to the client.

### Protocol Tests

Write several tests to verify the protocol is serialized and deserialized correctly. I'm not specifying a number of tests to write. Instead, these unit tests should verify the following functionality and behavior:

- Serializing and deserializing a `File` message
- Serializing and deserializing a `Request` message
- Serializing and deserializing a `Status` message
- Edge cases for both types of messages, such as empty filenames or files with no data.
- Malformed messages, such as messages that are too short or have incorrect tags, or don't conform to the Pack109 protocol.

🤖 **AI Usage Note** - Feel Free to use AI for this part to help you generate tests. Just provide a link to the AI conversation that generated the tests.

### Part 2 - File Server Implementation

Once you have the communication protocol implemented, you will implement the file server. The file server will listen for incoming connections from clients, and process messages according to the protocol defined above.

As you work on this part, you can use the provided client binary to test your implementation. Also, if you could not complete the first part fully, you can still do this part by using the example messages provided in the previous section -- sending those messages directly to the server using a tool like `netcat` or `telnet`, or your own custom socket client implementation.

#### Client

The client can be used with the following flags:

```
--hostname address:port
--send filename
--request filename
```

#### Server

There are three versions of the server you can implement,  depending on the time you have to devote to the exam, and your confidence / ambition. Each level has a different maximum possible score for this part of the exam.

You only have to do one. The easy version has more content from the beginning of the course, while the harder version are weighted more toward newer content.

- Easy (B 85%) - A basic server that can receive files and send them back to the client. 
    - Handles small files
    - Stores files in-memory and does not persist them, so they are lost when the server is terminated.
    - Handles one client connection at a time
- Medium (A 95%) - A more robust server that has more features
    - Handles larger files
    - Persists stored files to disk on exit, and loads them back into memory on startup
    - Still just handles one client connection at a time
- Hard (A+ 100%) - A fully-featured server that can handle multiple client connections at once
    - Handles larger files
    - Persists stored files to disk on exit, and loads them back into memory on startup
    - Handles multiple client connections at once using threads or asynchronous I/O

You can start in easy mode and work your way up to medium or hard if you have time, but this might requrie you to refactor your code as you go. Alternatively you can start in hard mode and work your way down if you find yourself getting stuck.

### 😸 Easy Server (B 85%)

**Max file size:** 255 bytes

The easy file server program must accept the following flag:

1. `--hostname -h address:port` - Optional flat Where `address` is a 32 bit IPv4 address, and `port` is the desired port of the file server.
    - If you call the program without this flag, it defaults to `localhost:8081`.

---

The easy file server program will implement the following functionality:

- Q1. The server will start, bind a socket to a port, and listen for a connection.
- Q2. The server will create an empty hash map to store received files in memory.
  - You have to write the hash map yourself. You can't use a library that you did not write. You can use your Homework code or the posted solution as a starting point. Remember to cite this code if you use it. If you look at any source for help, be sure to cite it.
  - The hash map will use the filename string as a key, and the file as a value (this can be either the received `File`, or the file data `vector<u8>`).
  - The hashmap will act much like the hash set, except it will have two additional methods:
    - Q3. `bool insert(String key, File value);` - inserts the k/v pair into the hash map. Returns true if the key already existed, and replaces the stored value with the supplied value. Returns false if the key did not exist already.
    - Q4. `File get(String key);` - Returns the file associated with the supplied key. If the key doesn't exist, this function throws an exception.
- When the server receives a connection, it will enter an infinite loop.
  - This loop will attempt to receive data from the client. When it receives a message, it will follow the following steps:
    - Q5. Read the message to a buffer
    - Q6. Decrypt the message using a simple XOR encryption scheme. The key is `42`.
    - Q7. Deserialize the message to the appropriate struct, either a `File` or `Request`.
      - Q8. If the message is a `File`, then the server will insert the filename and file into the hash map.
        - If the file is already in the hash map, it will replace the existing file with the new one and the response will be a `Status` message indicating that the file was replaced.
        - Otherwise the response will be a `Status` message indicating that the file was successfully stored.
        - If the file cannot be stored in the hash map due to memory issues, the resposne will be a `Status` message indicating that the file could not be stored.
      - Q9. If the message is a `Request`, then the server will look for the requested file in the hash map. 
        - If the requested file does not exist, the reposne will be a `Status` message indicating that the file was not found.
        - If the file does exist it will be serialized into a `File` message, encrypted
    - Q10. Send the response back to the client.
        - Encrypt the response using the same XOR encryption scheme with a key of `42`.
        - Serialize the response to a buffer.
        - Send the buffer back to the client.
  - After servicing a message, the file server will loop and wait for a new message from the client.
  - The file server will not terminate until the user terminates the program.

### 😼 Medium Server (A 95%)

**Max file size**: 65,535 bytes

The medium file server program must accept the following flags:

1. `--hostname -h address:port` - Optional flat Where `address` is a 32 bit IPv4 address, and `port` is the desired port of the file server.
    - If you call the program without this flag, it defaults to `localhost:8081`.

2. `--persist -p persist_filename` - Optional flag that specifies a file to persist the hash map to disk. 
    - If this flag is not provided, the hash map will not be persisted and the files will be lost when the server is terminated. 
    - If this file already exists, it will be read and the hash map will be populated with the contents of the file. 
    - If the file isn't in the right format, the server should terminate with an error message.

---

The medium server will implement all of the requirements of the easy server, with the following additions and modifications:

- Q11. The server will persist the hash map to disk when it is terminated.
    - The server will write the hash map to the specified file in a format that can be read back in later. 
    - You can decide what this format is, but a binary data format in the PACK109 protocol is a good choice. JSON or XML are also acceptable.
    - If the file cannot be written to disk due to permission issues or other errors, the server should terminate with an error message.
- Q12. The server will load the hash map from disk when it is started.
    - The server will read the hash map from the specified file if it exists, and populate the in-memory hash map with the contents of the file.
    - If the file cannot be read due to permission issues or other errors, the server should terminate with an error message.

### 🙀 Hard Server (A+ 100%)

**Max file size**: 65,535 bytes

The hard file server program must accept the following flags:

1. `--hostname -h address:port` - Optional flat Where `address` is a 32 bit IPv4 address, and `port` is the desired port of the file server.
    - If you call the program without this flag, it defaults to `localhost:8081`.

2. `--persist -p persist_filename` - Optional flag that specifies a file to persist the hash map to disk. 
    - If this flag is not provided, the hash map will not be persisted and the files will be lost when the server is terminated. 
    - If this file already exists, it will be read and the hash map will be populated with the contents of the file. 
    - If the file isn't in the right format, the server should terminate with an error message.

3. `--max-connections -m max_connections` - Optional flag that specifies the maximum number of concurrent connections the server will handle.
    - If this flag is not provided, the server will handle one connection at a time.

---

The hard server will implement all of the requirements of the medium server, with the following additions and modifications:

- Q13. The server will handle multiple client connections at once using threads or asynchronous I/O.
    - If the `--max-connections` flag is provided, the server will create a new process for each incoming connection up to the specified maximum number of connections.
    - If the maximum number of connections is reached, the server will refuse new connections until one of the existing connections is closed.
    - If the `--max-connections` flag is not provided, the server will handle one connection at a time as in the medium server.
    - You must determine how to share the hash map between the threads or processes. You can use mutexes, semaphores, [pipe](https://man7.org/linux/man-pages/man2/pipe.2.html), [mmap](https://man7.org/linux/man-pages/man2/mmap.2.html), or another socket.

### Part 3 - Video Explanation

This is the oral portion of the exam. You will record an explanation for your file server, and demonstrate that it works with a provided client.

To demonstrate your file server working, it's sufficient to show the client sending a file (any file) to the server, the server acknowledging its receipt, and then to show the client requesting it again and saving it to disk. For the medium and hard servers, you should also demonstrate that the files persist across server restarts, and multiple clients can connect to the server at once.

If you didn't get the file server fully working, explain how you attempted to solve this exam and where you got stuck. Show off any code you did write. This will get you full credit for this portion.

Here are some questions to answer during your video:

1. How did you handle errors and exceptions in your file server? Did you consider all possible error scenarios and how did you ensure that your file server gracefully handles them?

2. Did you consider the performance and efficiency of your file server in terms of file transfer speed and resource utilization? If so, what optimization techniques did you use to improve the performance of your file server? If not, what measures could you take to improve the efficiency of your file server?

3. Did you consider the possibility of the client sending malformed or malicious data to your file server? If so, what steps did you take to validate the data and prevent security vulnerabilities? If not, what measures could you take to improve the security of your file server?

You can use Zoom to do this, [here is a link][3] to some instructions. You don't have to record your face, only your voice and the screen. Go through your code and explain how you the important parts (important is subjective here. Usually the important bits are the ones you spent the most time on or had the most difficulty with). Your goal with this section is to convince me you know what you are talking about, so I want you to do this without reading a script or written answer. When you are done, upload your recording to your final exam repository.

**⚠️IMPORTANT: Make sure you give blanket permission to the link holder to view the file**

🎥 Paste Recording Link(s) Here:

## Submission

Please submit your completed exam, which should include:

1. Your protcol source code and tests
2. Your file server source code
3. A recording link with permission to view granted to the link holder.
4. No need to include binary / executable files.

- Only files under version control in your forked assignment repository will be graded. Local files left untracked on your computer will not be considered.

- Only code committed *and pushed* prior to the time of grading will be accepted. Locally committed but unpushed code will not be considered.

- Your assignment will be graded according to the [Programming Assignment Grading Rubric](https://drive.google.com/open?id=1V0nBt3Rz6uFMZ9mIaFioLF-48DFX0VdkbgRUDM_eIFk).

Your submission should be organized, well-commented, and easy to understand. Remember to document any assumptions you made during the implementation process, as well as any limitations of your solution. Your final exam will be graded on the correctness, completeness, and clarity of your submission.

## Works Cited

Include a numbered list of websites and resources you used to complete this exam. The format of the Works Cited does not matter, just be consistent and provide all the information needed to find the cited information. Include a number-reference (e.g. `[1]`) to the citation as close to where the material is used in the exam. If the resource is web-based, a link is sufficient.

[0]: https://studentaffairs.lehigh.edu/content/code-conduct
[2]: http://crasseux.com/books/ctutorial/Building-a-library.html#Building%20a%20library
[3]: https://support.zoom.us/hc/en-us/articles/201362473-Local-recording
