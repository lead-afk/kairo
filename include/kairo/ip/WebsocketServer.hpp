// #pragma once

// #include "kairo/ds/BiMap.hpp"
// #include <cstdint>
// #include <functional>
// #include <shared_mutex>
// #include <string>
// #include <thread>
// #include <unordered_map>
// #include <uwebsockets/App.h>

// namespace kairo::ip
// {

// using kairo::ds::Bimap;

// /**
//  * @brief Per-connection user data stored by uWebSockets.
//  *
//  * uWebSockets associates an instance of this struct with each WebSocket. We
//  use
//  * it to cache our numeric connection identifier (ws-id).
//  */
// struct PerSocketData
// {
//     /// @brief Numeric connection id (ws-id). May be injected via upgrade
//     query
//     /// `?id=<n>`.
//     uint64_t id{};
// };

// /**
//  * @brief Message payload tagged with the sender ws-id.
//  */
// struct IncomingMessage
// {
//     uint64_t client_id;
//     std::string message;
// };

// /**
//  * @brief WebSocket server wrapper around uWebSockets.
//  *
//  * Runs the uWS event loop on a dedicated thread, assigns a server UUID per
//  * client, and dispatches events via callbacks.
//  *
//  * Notes:
//  * - The uWS loop is owned by the server thread
//  * - `stop()` uses `loop->defer(...)` to close sockets on the loop thread
//  * - Public helpers use locks, but some uWS operations may require the loop
//  * thread
//  */
// class WebsocketServer
// {
//   public:
//     /**
//      * @brief Construct a new WebSocket server.
//      *
//      * @param port The port to listen on.
//      * @param on_message_callback Called when a text message arrives:
//      * (client_uuid, payload).
//      * @param on_client_connected_callback Called after a new client is
//      opened:
//      * (client_uuid, client_ip).
//      * @param on_client_disconnected_callback Called when a client
//      disconnects:
//      * (client_uuid).
//      */
//     explicit WebsocketServer(
//         int port,
//         std::function<void(const std::string &, const std::string &)>
//             on_message_callback,
//         std::function<void(const std::string &, const std::string &)>
//             on_client_connected_callback,
//         std::function<void(const std::string &)>
//             on_client_disconnected_callback);
//     /**
//      * @brief Destroy the server.
//      *
//      * Calls `stop()` to gracefully shut down if running.
//      */
//     ~WebsocketServer();

//     /**
//      * @brief Start the WebSocket server
//      *
//      * Spawns a dedicated thread that owns the uWS loop.
//      */
//     void start();

//     /**
//      * @brief Stop the WebSocket server gracefully
//      *
//      * Closes the listening socket and all client connections, then joins the
//      * server thread. Uses `loop->defer(...)` to perform close operations on
//      the
//      * loop thread.
//      */
//     void stop();

//     /**
//      * @brief Send a message to a specific client
//      *
//      * Sends a TEXT frame to the WebSocket identified by the numeric ws-id.
//      *
//      * @param client_id The ws-id to send to.
//      * @param message The message payload.
//      */
//     void send_message_by_ws_id(uint64_t client_id, const std::string
//     &message);

//     /**
//      * @brief Send a message to a specific client by UUID.
//      *
//      * Resolves UUID -> ws-id via the UUID map, then forwards to
//      * `send_message_by_ws_id`.
//      *
//      * @param client_uuid The server-assigned UUID for the client.
//      * @param message The message payload.
//      */
//     void send_message_by_uuid(const std::string &client_uuid,
//                               std::string message);

//     /**
//      * @brief Check if a UUID is currently connected.
//      * @param client_uuid The server-assigned UUID.
//      * @return true if the UUID is present in the UUID<->ws-id map.
//      */
//     bool is_connected_uuid(const std::string &client_uuid);

//     /**
//      * @brief Disconnect a client by UUID.
//      *
//      * Looks up UUID -> ws-id -> socket and calls `close()`.
//      *
//      * @param client_uuid The server-assigned UUID.
//      */
//     void disconnect_client_by_uuid(const std::string &client_uuid);

//   private:
//     /// @brief TCP port to listen on.
//     int port;

//     /// @brief Dedicated thread that owns the uWS loop.
//     std::thread thread;

//     /// @brief Guards `clients`.
//     std::shared_mutex clients_mutex;

//     /// @brief Map: ws-id (numeric connection id) -> raw uWS WebSocket
//     pointer. std::unordered_map<uint64_t, ::uWS::WebSocket<false, true,
//     PerSocketData> *>
//         clients;

//     /// @brief uWS event loop pointer (owned/used by the server thread).
//     ::uWS::Loop *loop{nullptr};

//     /// @brief uSockets listen handle
//     /// Used to stop accepting new connections
//     struct us_listen_socket_t *listen_token{nullptr};

//     /// @brief Guards `client_uuid_wsid_map`.
//     std::shared_mutex client_uuid_wsid_map_mutex;

//     /// @brief BiMap: client_uuid (string) <-> ws-id (uint64_t).
//     ///
//     /// - Key: UUID assigned by `handle_open()`.
//     /// - Value: ws-id stored in `PerSocketData::id`.
//     Bimap<std::string, uint64_t> client_uuid_wsid_map;

//     /// @brief Called when a message is received: (client_uuid, message).
//     std::function<void(const std::string &, const std::string &)>
//         on_message_callback;

//     /// @brief Called when a client is connected: (client_uuid, client_ip).
//     std::function<void(const std::string &, const std::string &)>
//         on_client_connected_callback;

//     /// @brief Called when a client disconnects: (client_uuid).
//     std::function<void(const std::string &)> on_client_disconnected_callback;

//     /// @brief Entrypoint for the server thread, configures uWS callbacks and
//     /// runs the loop.
//     void run_loop();

//     /// @brief uWS upgrade handler
//     /// Seeds PerSocketData id from the optional query `id`
//     void handle_upgrade(auto *res, auto *req, auto *context);

//     /// @brief uWS open handler
//     /// Assigns a ws-id, creates a UUID, and registers UUID <-> ws-id
//     mappings
//     /// @return The newly generated client UUID.
//     std::string handle_open(auto *ws, std::atomic_uint64_t &next_id);

//     /// @brief uWS message handler.
//     /// Resolves ws-id -> UUID and dispatches `on_message_callback` on a
//     /// detached thread.
//     void handle_message(auto *ws, std::string_view message,
//                         ::uWS::OpCode op_code);

//     /// @brief uWS close handler.
//     /// Cleans up client state and dispatches
//     `on_client_disconnected_callback`. void handle_close(auto *ws, int code,
//     std::string_view msg);
// };
// } // namespace kairo::ip