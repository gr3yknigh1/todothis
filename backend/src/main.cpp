/*
 * Main server of `todothis`
 * */
#include <cstdint>
#include <cstdlib>

#include <httplib.h>
#include <spdlog/spdlog.h>

#define _DEFAULT_PORT 8080

int main(int argc, char *argv[])
{
    auto logger = spdlog::default_logger();
    logger->set_level(spdlog::level::trace);

    logger->debug("Running with arguments:");
    for (int i = 0; i < argc; ++i)
    {
        logger->debug("> [{}]: {}", i, argv[i]);
    }

    enum Args {
        EXEC,
        PORT,
        COUNT,
    };

    uint16_t server_port = _DEFAULT_PORT;

    if (argc >= Args::COUNT) {
        server_port = std::atoi(argv[Args::PORT]);
    }

    logger->info("Initalizing server...");

    httplib::Server server;

    server.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });

    logger->info("Starting server...");

    server.listen("0.0.0.0", server_port);

    logger->info("Stopping server...");
    return EXIT_SUCCESS;
}
