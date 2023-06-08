/*
 * Main server of `todothis`
 * */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define RET_ERR (-1)
#define LISTEN_PORT 8080
#define CONNECTION_TRYIES 5

int main()
{
    int ret_code, on = 1;

    // NOTE: Creating socket
    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd <= RET_ERR)
    {
        std::fprintf(stderr, "socket() failed: %i\n", socket_fd);
        std::exit(EXIT_FAILURE);
    }

    // NOTE: Setting socket to be reusable
    ret_code = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret_code == RET_ERR)
    {
        std::fprintf(stderr, "setsockopt() failed: %i\n", ret_code);
        std::exit(EXIT_FAILURE);
    }

    // NOTE: Bind socket
    sockaddr_in6 addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    std::memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port = htons(LISTEN_PORT);

    ret_code = bind(socket_fd, (sockaddr *)&addr, sizeof(addr));
    if (ret_code == RET_ERR)
    {
        std::fprintf(stderr, "bind() failed: %i\n", ret_code);
        close(socket_fd);
        std::exit(EXIT_FAILURE);
    }

    ret_code = listen(socket_fd, CONNECTION_TRYIES);
    if (ret_code == RET_ERR)
    {
        std::fprintf(stderr, "listen() failed: %i\n", ret_code);
        close(socket_fd);
        std::exit(EXIT_FAILURE);
    }

    std::fprintf(stdout, "Server successfully binded to %i\n", LISTEN_PORT);

    while (true)
    {
        std::fprintf(stdout, "Waiting for request...\n");
        int connection_fd = accept(socket_fd, NULL, NULL);

        if (connection_fd == RET_ERR)
        {
            std::fprintf(stderr, "accept() failed: %i\n", connection_fd);
            continue;
        }

        std::fprintf(stdout, "Accepted request!\n");

        char req_buffer[1024];
        ret_code = static_cast<int>(
            recv(connection_fd, req_buffer, sizeof(req_buffer), 0));
        if (ret_code == RET_ERR)
        {
            std::fprintf(stderr, "recv() failed: %i\n", ret_code);
            continue;
        }

        std::fprintf(stdout, "Request:\n%s\n\n", req_buffer);

        char res_buffer[1024] = "HTTP/1.1 200 OK\r\nContent-Type: "
                                "text/plain\r\nContent-Length: 5\r\n\r\nHihi\n";
        std::fprintf(stdout, "Response:\n%s\n\n", res_buffer);
        ret_code = static_cast<int>(
            send(connection_fd, res_buffer, sizeof(res_buffer), 0));
        if (ret_code == RET_ERR)
        {
            std::fprintf(stderr, "send() failed: %i\n", ret_code);
            close(socket_fd);
            close(connection_fd);
            return EXIT_FAILURE;
        }

        close(connection_fd);
        break;
    }

    close(socket_fd);
    return EXIT_SUCCESS;
}
