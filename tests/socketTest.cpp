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

#define MAX_CLIENTS 30
#define TIMEOUT_MICROSECONDS 1

class SocketServer
{
public:
    SocketServer()
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = TIMEOUT_MICROSECONDS;

        // initialise all client_socket[] to 0 so not checked
        for (int i = 0; i < MAX_CLIENTS; i++)
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
        int opt = 1;
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
        int addrlen = sizeof(address);

        spdlog::info("Waiting for connections ...");
        while (1)
        {
            // set of socket descriptors for sockets with data to be read.
            fd_set readfds;
            // set of socket descriptors for sockets that can be written to.
            fd_set writefds;

            int max_sd = getMaxClientID(&client_socket);
            initFDSet(&readfds, &client_socket);
            initFDSet(&writefds, &client_socket);

            int activity = select(max_sd + 1, &readfds, &writefds, NULL, &timeout);

            if ((activity < 0) && (errno != EINTR))
            {
                spdlog::error("select error");
            }

            int new_socket = acceptNewConn(&readfds);

            if (new_socket > 0)
            {
                // inform user of socket number - used in send and receive commands
                DEBUG("New connection , socket fd is {} , ip is : {} , port : {}", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                // send new connection greeting message
                if (send(new_socket, message, strlen(message), 0) != strlen(message))
                {
                    spdlog::error("send");
                }

                DEBUG("Welcome message sent successfully");
            }

            // else its some IO operation on some other socket
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                int sd = client_socket[i];

                if (FD_ISSET(sd, &readfds))
                {
                    int valread = handleErrors(i, &readfds);

                    if (valread > 0)
                    {
                        // Old code to send data to one client.
                        // send(sd, buffer, strlen(buffer), 0);

                        // set the string terminating NULL byte on the end
                        // of the data read
                        buffer[valread] = '\0';
                        for (int j = 0; j < MAX_CLIENTS; j++)
                        {
                            send(client_socket[j], buffer, strlen(buffer), 0);
                        }
                    }
                }

                // Constantly send the string "asdf" to all connected clients to test streaming data.
                if (FD_ISSET(sd, &writefds))
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
    int master_socket, client_socket[MAX_CLIENTS];
    struct sockaddr_in address;

    char buffer[1025]; // data buffer of 1K

    // Use non-blocking sockets to wait for activity. Only wait for 1 microsecond.
    struct timeval timeout;

    // a message
    char *message = "ECHO Daemon v1.0 \r\n";

    int getMaxClientID(int (*client_socket)[MAX_CLIENTS])
    {
        // Return the largest client socket id of all client sockets.
        int max_sd = master_socket;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = (*client_socket)[i];
            if (sd > max_sd)
                max_sd = sd;
        }

        return max_sd;
    }

    int initFDSet(fd_set *fds, int (*client_socket)[MAX_CLIENTS])
    {
        // clear the socket set
        FD_ZERO(fds);

        // add master socket to set
        FD_SET(master_socket, fds);

        // add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            // socket descriptor
            int sd = (*client_socket)[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, fds);
        }
    }

    int acceptNewConn(fd_set *readfds)
    {
        int new_socket = 0;
        int addrlen = sizeof(address);

        if (FD_ISSET(master_socket, readfds))
        {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address,
                                     (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++)
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

        return new_socket;
    }

    // @TODO make buffer a pointer to this
    int handleErrors(int i, fd_set *readfds)
    {
        int valread;
        int addrlen = sizeof(address);
        int sd = client_socket[i];

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
        }
        else if (valread == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // The socket does not have any data to be read. Try again later.
                // @TODO may not be necesary if we are using non-blocking sockets
                // because we will only get data for sockets that have data.
                return valread;
            }
            else
            {
                // @TODO handle other errors here
                return valread;
            }
        }

        return valread;
    }
};

int main(int argc, char *argv[])
{
    SocketServer server;
    server.bindSocket(8888);
    server.listenToSocket();

    return 0;
}
