/*
 * Main server of `todothis`
 * */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>

#include <netinet/in.h>
#include <optional>
#include <sys/socket.h>
#include <unistd.h>

#include <sqlite3.h>

#define RET_ERR (-1)
#define LISTEN_PORT 8080
#define CONNECTION_TRYIES 5

#define TT_ASSERT_SQLITE_ERR(__DB, __RC)                                       \
    do                                                                         \
    {                                                                          \
        if ((__RC) != SQLITE_OK)                                               \
        {                                                                      \
            std::fprintf(                                                      \
                stderr,                                                        \
                "AssertionError at %s:%i: Failed call sqlite3 function\n",     \
                __FILE__, __LINE__);                                           \
            std::fprintf(stderr, "%s\n", sqlite3_errmsg(__DB));                \
            std::exit(EXIT_FAILURE);                                           \
        }                                                                      \
    } while (0)

#define TT_STRINGIFY(X) #X

namespace
{

static void PrintUsage()
{
    std::cerr << "Usage: todothis <path/to/sqlite3/db>\n";
}

[[maybe_unused]] static int SQLite3StmtExecCallback(void *data, int argc,
                                                    char **argv,
                                                    char **azColName)
{
    (void)data;

    for (int i = 0; i < argc; i++)
    {
        std::printf("%s = %s\n", azColName[i],
                    argv[i] != nullptr ? argv[i] : "NULL");
    }
    std::printf("\n");

    return SQLITE_OK;
}

[[maybe_unused]] static bool IsSQLiteTableExists(sqlite3 *db,
                                                 const char *tableName)
{
    const std::string statement =
        "SELECT name FROM sqlite_master WHERE type='table' AND name='" +
        std::string(tableName) + "';";

    bool isTableExists = false;

    const auto callback = [](void *data, int, char **, char **) -> int {
        *(bool *)data = true;
        return 0;
    };

    int rc =
        sqlite3_exec(db, statement.c_str(), callback, &isTableExists, NULL);
    TT_ASSERT_SQLITE_ERR(db, rc);

    return isTableExists;
}

[[maybe_unused]] static void DropTable(sqlite3 *db, const char *tableName)
{
    const std::string sql{"DROP TABLE " + std::string(tableName) + ';'};
    int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
    TT_ASSERT_SQLITE_ERR(db, rc);
}

[[maybe_unused]] static void CreateUsersTableIfNotExist(sqlite3 *db)
{
    const std::string sql{"CREATE TABLE IF NOT EXISTS users ( "
                          "   id INTEGER PRIMARY KEY, "
                          "   first_name TEXT, "
                          "   last_name TEXT, "
                          "   email TEXT "
                          "); "};
    int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
    TT_ASSERT_SQLITE_ERR(db, rc);
}

enum class HTTPStatus : uint16_t
{
    OK = 200,
};

std::ostream &operator<<(std::ostream &stream, HTTPStatus status)
{
    return stream << static_cast<uint16_t>(status);
}

[[maybe_unused]] static const char *HTTPStatusToString(HTTPStatus status)
{
    switch (status)
    {
    case HTTPStatus::OK:
        return TT_STRINGIFY(OK);
        break;
    }
    return "NULL";
}

struct HTTTResponse
{
    HTTPStatus status;
    std::string type;
    std::string body;
};

[[maybe_unused]] static std::string MakeRawHTTPRequest(HTTPStatus status)
{
    std::stringstream ss;
    ss << "HTTP/1.1 " << status << " " << HTTPStatusToString(status) << "\r\n";

    return ss.str();
}

}; // namespace

int main(int argc, char *argv[])
{
    enum Arg
    {
        EXEC_PATH,
        DB_PATH,
    };

    if (argc <= 1)
    {
        std::cerr << "Error: wrong number of arguments\n";
        PrintUsage();
        return EXIT_FAILURE;
    }

    std::filesystem::path db_path{argv[Arg::DB_PATH]};

    if (std::filesystem::exists(db_path) &&
        !std::filesystem::is_regular_file(db_path))
    {
        std::fprintf(stderr, "'%s' doesn't a file\n", db_path.c_str());
        return EXIT_FAILURE;
    }

    int rc;

    sqlite3 *db = NULL;
    rc = sqlite3_open_v2(db_path.c_str(), &db,
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    TT_ASSERT_SQLITE_ERR(db, rc);

    CreateUsersTableIfNotExist(db);
    // DropTable(db, "users");

    // NOTE: Creating socket
    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd <= RET_ERR)
    {
        std::fprintf(stderr, "socket() failed: %i\n", socket_fd);
        std::exit(EXIT_FAILURE);
    }

    // NOTE: Setting socket to be reusable
    int on = 1;
    rc = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (rc == RET_ERR)
    {
        std::fprintf(stderr, "setsockopt() failed: %i\n", rc);
        std::exit(EXIT_FAILURE);
    }

    // NOTE: Bind socket
    sockaddr_in6 addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    std::memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port = htons(LISTEN_PORT);

    rc = bind(socket_fd, (sockaddr *)&addr, sizeof(addr));
    if (rc == RET_ERR)
    {
        std::fprintf(stderr, "bind() failed: %i\n", rc);
        close(socket_fd);
        std::exit(EXIT_FAILURE);
    }

    rc = listen(socket_fd, CONNECTION_TRYIES);
    if (rc == RET_ERR)
    {
        std::fprintf(stderr, "listen() failed: %i\n", rc);
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
        rc = static_cast<int>(
            recv(connection_fd, req_buffer, sizeof(req_buffer), 0));
        if (rc == RET_ERR)
        {
            std::fprintf(stderr, "recv() failed: %i\n", rc);
            continue;
        }

        // std::fprintf(stdout, "Request:\n%s\n\n", req_buffer);

        char res_buffer[1024] = "HTTP/1.1 200 OK\r\nContent-Type: "
                                "text/plain\r\nContent-Length: 5\r\n\r\nHihi\n";
        rc = static_cast<int>(
            send(connection_fd, res_buffer, sizeof(res_buffer), 0));
        if (rc == RET_ERR)
        {
            std::fprintf(stderr, "send() failed: %i\n", rc);
            close(socket_fd);
            close(connection_fd);
            return EXIT_FAILURE;
        }

        close(connection_fd);
    }

    close(socket_fd);

    sqlite3_close(db);
    return EXIT_SUCCESS;
}
