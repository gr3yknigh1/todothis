/*
 * Main server of `todothis`
 * */
#include <cstdint>
#include <cstdlib>

#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
    auto logger = spdlog::default_logger();
    logger->set_level(spdlog::level::trace);

    logger->debug("Running with arguments:");
    for (int i = 0; i < argc; ++i)
    {
        logger->debug("> [{}]: {}", i, argv[i]);
    }
    logger->info("Starting server...");

    logger->info("Stopping server...");
    return EXIT_SUCCESS;
}
