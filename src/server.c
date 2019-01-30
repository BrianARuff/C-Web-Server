/**
 * webserver.c -- A webserver written in C
 *
 * Test with curl (if you don't have it, install it):
 *
 *    curl -D - http://localhost:3490/
 *    curl -D - http://localhost:3490/d20
 *    curl -D - http://localhost:3490/date
 *
 * You can also test the above URLs in your browser! They should work!
 *
 * Posting Data:
 *
 *    curl -D - -X POST -H 'Content-Type: text/plain' -d 'Hello, sample data!' http://localhost:3490/save
 *
 * (Posting data is harder to test from a browser.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include "file.h"
#include "mime.h"
#include "cache.h"

#define PORT "3490"  // the port users will be connecting to

#define SERVER_FILES "./serverfiles"
#define SERVER_ROOT "./serverroot"

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 *
 * Return the value from the send() function.
 */
int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{
    // const int max_response_size = 65536;
    const int max_response_size = 180000;
    char response[max_response_size];

    // Get the current time in seconds
    // time_t == time type to variable named seconds
    time_t seconds;
    // struct tm with a pointer to info variable, tm is the structure for time that contains sec, min, hour, day of month, mont hof year, years since, day of week, etc...
    struct tm *info;

    // time function called with seconds address
    // time since 01/01/1970 UTC 00:00:00
    time(&seconds);
    // info is localtime function called with second address
    // gives us local time
    info = localtime(&seconds);

    // string var called body_str set to the variable "body"'s value
    char *body_str = body;

    // Format HTTP response
    int response_length = sprintf( response,
      "%s\n"
      "Date: %s"
      "Connection: close\n"
      "Content-Length: %d\n"
      "Content-Type: %s\n"
      "\n",
      header, // passed in
      asctime(info), // take info and turns it into asctime which is more readable (Sat Mar 25 06:10:10 1989)
      content_length, // passed in
      content_type); // passed in

      printf("Response length: %d\n", response_length + content_length);

     // passes body characters to the memory area of response+response_length, and content_length is the number of bytes to copy over
      memcpy(response + response_length, body, content_length);
      response_length += content_length;

    // Build HTTP response and store it in response

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Send it all!
    int rv = send(fd, response, response_length, 0);

    if (rv < 0) {
        perror("send");
    }

    return rv;
}


/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd)
{
    // Generate a random number between 1 and 20 inclusive
    // Set the random seed based on time
    srand(time(NULL));

    // Generate random number
    int randomNumber = rand() % 21;

    // Body of the response to send
    char body[8];

    // Format the body
    sprintf(body, "%d\n", randomNumber);



    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Use send_response() to send it back as text/plain data
    // Send the response
    send_response(fd, "HTTP/1.1 200 ok", "text/plain", body, strlen(body));

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
}

/**
 * Send a 404 response
 */
void resp_404(int fd)
{
    char filepath[4096];
    struct file_data *filedata;
    char *mime_type;

    // Fetch the 404.html file
    snprintf(filepath, sizeof filepath, "%s/404.html", SERVER_FILES);
    filedata = file_load(filepath);

    if (filedata == NULL) {
        // TODO: make this non-fatal
        fprintf(stderr, "cannot find system 404 file\n");
        exit(3);
    }

    mime_type = mime_type_get(filepath);

    send_response(fd, "HTTP/1.1 404 NOT FOUND", mime_type, filedata->data, filedata->size);

    file_free(filedata);
}

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, struct cache *cache, char *request_path)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    char file_path[256];
    struct file_data *file_body = NULL;
    char *mime_type;

    // Format the file
    sprintf(file_path, "./serverroot%s", request_path);
    // snprintf(file_path, sizeof file_path, "%s%s", SERVER_ROOT, request_path);

    // Set the body to the file's contents
    file_body = file_load(file_path);

    // If there isn't anything loaded, send a 404 not found,
    // otherwise, send the file's contents
    if (file_body == NULL) {
      resp_404(fd);
    } else {

      // Set the MIME
      mime_type = mime_type_get(file_path);

      // Send the file
      send_response(fd, "HTTP/1.1 200 OK", mime_type, file_body->data, file_body->size);

      // Free the file
      file_free(file_body);
    }


}

/**
 * Search for the end of the HTTP header
 *
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
    ///////////////////
    // IMPLEMENT ME! // (Stretch)
    ///////////////////
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];
    char method[8];
    char path[32];
    char protocol[16];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }


    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Read the three components of the first request line
    sscanf(request, "%s %s %s", method, path, protocol);

    printf("PATH -> %s\n", path);

    // If GET, handle the get endpoints
    if (strcmp(method, "GET") == 0 && strcmp(path, "d20") == 0) {
      get_d20(fd);
    } else if (strcmp(method, "GET") == 0) {
      get_file(fd, cache, path);
    } else {
      resp_404(fd);
    }


    // (Stretch) If POST, handle the post request
}

/**
 * Main
 */
int main(void)
{
    int newfd;  // listen on sock_fd, new connection on newfd
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];

    struct cache *cache = cache_create(10, 0);

    // Get a listening socket
    int listenfd = get_listener_socket(PORT);

    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections on port %s...\n", PORT);

    // This is the main loop that accepts incoming connections and
    // forks a handler process to take care of it. The main parent
    // process then goes back to waiting for new connections.

    while(1) {
        socklen_t sin_size = sizeof their_addr;

        // Parent process will block on the accept() call until someone
        // makes a new connection:
        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd == -1) {
            perror("accept");
            continue;
        }

        // Print out a message that we got the connection
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        // newfd is a new socket descriptor for the new connection.
        // listenfd is still listening for new connections.

        handle_http_request(newfd, cache);

        close(newfd);
    }

    // Unreachable code

    return 0;
}