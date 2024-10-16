#include <string>
#include <utility>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/server.hpp"

using namespace opcua;

TEST_CASE_TEMPLATE("Config", T, ClientConfig, ServerConfig) {
    using NativeType = typename T::NativeType;

    SUBCASE("Default constructor") {
        T config;
    }
    SUBCASE("Construct from native") {
        NativeType native{};
        T config(std::move(native));
    }
    SUBCASE("Move constructor") {
        T other;
        T config(std::move(other));
    }
    SUBCASE("Move assignment") {
        T other;
        T config = std::move(other);
    }

    T config;

    SUBCASE("setLogger") {
        static size_t counter = 0;
        static LogLevel lastLogLevel{};
        static LogCategory lastLogCategory{};
        static std::string lastMessage{};

        config.setLogger([&](LogLevel level, LogCategory category, std::string_view message) {
            counter++;
            lastLogLevel = level;
            lastLogCategory = category;
            lastMessage = message;
        });

        // passing a nullptr should do nothing
        config.setLogger(nullptr);

        UA_LOG_INFO(detail::getLogger(config.handle()), UA_LOGCATEGORY_USERLAND, "Message");
        CHECK(counter == 1);
        CHECK(lastLogLevel == LogLevel::Info);
        CHECK(lastLogCategory == LogCategory::Userland);
        CHECK(lastMessage == "Message");
    }

    SUBCASE("handle") {
        CHECK(config.handle() != nullptr);
        CHECK(std::as_const(config).handle() != nullptr);
    }
}

TEST_CASE_TEMPLATE("Connection", T, Client, Server) {
    // TODO: provide type alias NativeType
    using NativeType = std::remove_pointer_t<decltype(std::declval<T>().handle())>;

    SUBCASE("Default constructor") {
        T connection;
    }
    SUBCASE("Move constructor") {
        T other;
        T connection(std::move(other));
    }
    SUBCASE("Move assignment") {
        T other;
        T connection = std::move(other);
    }

    T connection;

    SUBCASE("config") {
        auto& config = detail::getConfig(connection);
        CHECK(connection.config().handle() == &config);
        CHECK(std::as_const(connection).config().handle() == &config);
    }

    SUBCASE("handle") {
        CHECK(connection.handle() != nullptr);
        CHECK(std::as_const(connection).handle() != nullptr);
    }

    SUBCASE("setCustomDataTypes") {
        auto& config = detail::getConfig(connection);
        CHECK(config.customDataTypes == nullptr);

        connection.setCustomDataTypes({
            DataType(UA_TYPES[UA_TYPES_STRING]),
            DataType(UA_TYPES[UA_TYPES_INT32]),
        });
        CHECK(config.customDataTypes != nullptr);
        CHECK(config.customDataTypes->next == nullptr);
        CHECK(config.customDataTypes->typesSize == 2);
        CHECK(config.customDataTypes->types != nullptr);
        CHECK(config.customDataTypes->types[0] == UA_TYPES[UA_TYPES_STRING]);
        CHECK(config.customDataTypes->types[1] == UA_TYPES[UA_TYPES_INT32]);
        
    }

    SUBCASE("Helper functions") {
        NativeType* nativeNull{nullptr};

        CHECK(detail::getConfig(nativeNull) == nullptr);
        CHECK(detail::getConfig(connection.handle()) != nullptr);
        CHECK(detail::getConfig(connection.handle()) == &detail::getConfig(connection));

        CHECK(detail::getConnection(nativeNull) == nullptr);
        CHECK(detail::getConnection(connection.handle()) != nullptr);
        CHECK(detail::getConnection(connection.handle()) == &detail::getConnection(connection));

        CHECK(detail::getWrapper(nativeNull) == nullptr);
        CHECK(detail::getWrapper(connection.handle()) != nullptr);
        CHECK(detail::getWrapper(connection.handle())->handle() == connection.handle());

        CHECK(detail::getContext(nativeNull) == nullptr);
        CHECK(detail::getContext(connection.handle()) != nullptr);
        CHECK(detail::getContext(connection.handle()) == &detail::getContext(connection));
    }

    SUBCASE("Equality operators") {
        T other;
        CHECK(connection == connection);
        CHECK(connection != other);
        CHECK(other == other);
    }
}