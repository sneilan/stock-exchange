// from https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
// Example code: A simple server side code, which echos back the received message.
// Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include "../util/spdlog/spdlog.h"

// Had some issues with spdlog levels. Will fix later.
#define DEBUG spdlog::info

class SocketServer
{
public:
    SocketServer()
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 1; // set the timeout to 1 microseconds

        // initialise all client_socket[] to 0 so not checked
        for (i = 0; i < max_clients; i++)
        {
            client_socket[i] = 0;
        }
    }

    void bindSocket(int PORT)
    {
        // create a master socket
        if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // set master socket to allow multiple connections ,
        // this is just a good habit, it will work without this
        if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                       sizeof(opt)) < 0)
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        // type of socket created
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // bind the socket to localhost port 8888
        if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        spdlog::info("Listener on port {}", PORT);
        // try to specify maximum of 3 pending connections for the master socket
        if (listen(master_socket, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    void listenToSocket()
    {
        // accept the incoming connection
        addrlen = sizeof(address);
        spdlog::info("Waiting for connections ...");
        while (1)
        {
            // clear the socket set
            FD_ZERO(&readfds);

            // add master socket to set
            FD_SET(master_socket, &readfds);
            max_sd = master_socket;

            // add child sockets to set
            for (i = 0; i < max_clients; i++)
            {
                // socket descriptor
                sd = client_socket[i];

                // if valid socket descriptor then add to read list
                if (sd > 0)
                    FD_SET(sd, &readfds);

                // highest file descriptor number, need it for the select function
                if (sd > max_sd)
                    max_sd = sd;
            }

            // I may need to 
            activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

            if ((activity < 0) && (errno != EINTR))
            {
                printf("select error");
            }

            // If something happened on the master socket ,
            // then its an incoming connection
            if (FD_ISSET(master_socket, &readfds))
            {
                if ((new_socket = accept(master_socket,
                                         (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                // inform user of socket number - used in send and receive commands
                DEBUG("New connection , socket fd is {} , ip is : {} , port : {}", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                // send new connection greeting message
                if (send(new_socket, message, strlen(message), 0) != strlen(message))
                {
                    perror("send");
                }

                DEBUG("Welcome message sent successfully");

                // add new socket to array of sockets
                for (i = 0; i < max_clients; i++)
                {
                    // if position is empty
                    if (client_socket[i] == 0)
                    {
                        client_socket[i] = new_socket;
                        DEBUG("Adding to list of sockets as {}", i);

                        break;
                    }
                }
            }

            // else its some IO operation on some other socket
            for (i = 0; i < max_clients; i++)
            {
                sd = client_socket[i];

                if (FD_ISSET(sd, &readfds))
                {
                    // Check if it was for closing , and also read the
                    // incoming message
                    if ((valread = read(sd, buffer, 1024)) == 0)
                    {
                        // Somebody disconnected , get the details and print
                        getpeername(sd, (struct sockaddr *)&address,
                                    (socklen_t *)&addrlen);
                        DEBUG("Host disconnected, ip {}, port {}",
                              inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                        // Close the socket and mark as 0 in list for reuse
                        close(sd);
                        client_socket[i] = 0;
                    } else if (valread == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // The socket does not have any data to be read. Try again later.
                            // @TODO may not be necesary if we are using non-blocking sockets
                            // because we will only get data for sockets that have data.
                            continue;
                        } else {
                            // handle other errors.
                            continue;
                        }
                    }
                    // Echo back the message that came in to all clients.
                    else
                    {
                        // Old code to send data to one client.
                        // send(sd, buffer, strlen(buffer), 0);

                        // set the string terminating NULL byte on the end
                        // of the data read
                        buffer[valread] = '\0';
                        for (int j = 0; j < max_clients; j++)
                        {
                            if (j == i)
                            {
                                continue;
                            }
                            send(client_socket[j], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }

            // Constantly send the string "asdf" to all connected clients to test streaming data.
            for (i = 0; i < max_clients; i++)
            {
                sd = client_socket[i];

                // Something about FD_ISSET is not letting me send to the client.
                // It's saying that after I send some data I can't send again.
                // I think I need to check how the select statement is working.
                if (FD_ISSET(sd, &readfds))
                {
                    const char *str = "asdf\n";
                    char arr[6];
                    strcpy(arr, str);
                    // @TODO fix this so we don't call send in a blocking manner.
                    send(sd, arr, strlen(arr), 0);
                }
            }
        }
    }

private:
    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[30],
        max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025]; // data buffer of 1K

    // set of socket descriptors
    fd_set readfds;
    // @TODO create a socket descriptor set of sockets to be written to
    // and maybe ones with errors.

    // Use non-blocking sockets to wait for activity. Only wait for 1 microsecond.
    struct timeval timeout;

    // a message
    char *message = "ECHO Daemon v1.0 \r\n";
};

int main(int argc, char *argv[])
{
    SocketServer server;
    server.bindSocket(8888);
    server.listenToSocket();

    return 0;
}
