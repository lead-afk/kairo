// #pragma once

// // #include "shared/connections/websocket/InterfaceWebsocketClient.hpp"
// #include <atomic>
// #include <deque>
// #include <functional>
// #include <mutex>
// #include <semaphore>
// #include <shared_mutex>
// #include <string>
// #include <thread>

// // using libchat::shared::connections::websocket::InterfaceWebsocketClient;

// // Forward declarations from libusockets (C structs live in the global
// // namespace). Do NOT declare these inside the libchat namespace, or they
// become
// // different types.
// struct us_socket_t;
// struct us_loop_t;
// namespace kairo::ip
// {

// // Forward declaration for uWebSockets
// namespace uWS
// {
// class Loop;
// }

// static ::us_socket_t *on_open(::us_socket_t *s, int is_client, char *ip,
//                               int ip_length);
// static ::us_socket_t *on_data(::us_socket_t *s, char *data, int length);
// static ::us_socket_t *on_close(::us_socket_t *s, int code, void *reason);
// static ::us_socket_t *on_writable(::us_socket_t *s);
// static ::us_socket_t *on_timeout(::us_socket_t *s);
// static ::us_socket_t *on_connect_error(::us_socket_t *s, int code);

// static void on_wakeup(::us_loop_t *loop);
// class WebsocketClient
// {
//   public:
//     /**
//      * @brief Construct a new Websocket Client object b
//      *
//      * @param uri The URI of the WebSocket server to connect to.
//      * @param client Reference to the delegate client handling incoming
//      * messages.
//      */
//     WebsocketClient(const std::string &uri,
//                     std::function<void(const std::string)>
//                     on_message_callback, std::function<void()>
//                     on_connect_callback, std::function<void()>
//                     on_disconnect_callback);
//     /**
//      * @brief Destroy the Websocket Client object
//      * Disconnects and cleans up resources.
//      */
//     ~WebsocketClient();

//     /**
//      * @brief Connect to the WebSocket server
//      * Starts the client loop in a separate thread.
//      */
//     void connect();

//     /**
//      * @brief Disconnect from the WebSocket server
//      * Stops the client loop and closes the connection.
//      */
//     void disconnect();

//     /**
//      * @brief Send a message to the server
//      *
//      * @param message The message content to send.
//      */
//     void send_message(const std::string &message);

//     /**
//      * @brief Check if the client is connected
//      *
//      * @return true if connected, false otherwise.
//      */
//     bool is_connected() const;

//     /**
//      * @brief Get the configured websocket endpoint URI
//      */
//     const std::string &get_uri() const { return uri; }

//     void set_connection_retry_count(int count)
//     {
//         std::unique_lock<std::shared_mutex>
//         lock(connection_retry_count_mutex); connection_retry_count = count;
//     }

//   private:
//     std::string uri;
//     std::thread client_thread;
//     std::thread heartbeat_thread;
//     std::atomic<bool> connected{false};
//     std::atomic<bool> should_stop{false};
//     std::mutex heartbeat_mutex;
//     std::binary_semaphore heartbeat_semaphore{
//         0}; // Used to stop the heartbeat loop
//     bool heartbeat_stop = false;
//     bool heartbeat_running = false; // True while the heartbeat thread is
//     active void heartbeat_loop();

//     // InterfaceWebsocketClient& iwebsocket_delegate;
//     std::function<void(const std::string)> on_message_callback;
//     std::function<void()> on_disconnect_callback;
//     std::function<void()> on_connect_callback;

//     std::deque<std::string> outgoing_queue;
//     std::mutex outgoing_mutex;

//     void *loop = nullptr;   // struct us_loop_t*
//     void *socket = nullptr; // struct us_socket_t*

//     bool main_loop_ready_to_join = false;
//     std::mutex main_loop_ready_to_join_mutex;
//     void run_loop();
//     void start_heartbeat();
//     void stop_heartbeat();

//     std::shared_mutex connection_retry_count_mutex;
//     int connection_retry_count = 10;

//     // Internal callbacks
//     void set_connected(bool state);
//     void on_connect() noexcept;
//     void on_disconnect() noexcept;
//     void on_message(const std::string message);
//     void set_socket(void *s);
//     void process_outgoing_queue();

//     friend ::us_socket_t *on_data(::us_socket_t *s, char *data, int length);
//     friend ::us_socket_t *on_open(::us_socket_t *s, int is_client, char *ip,
//                                   int ip_length);
//     friend ::us_socket_t *on_close(::us_socket_t *s, int code, void *reason);
//     friend ::us_socket_t *on_writable(::us_socket_t *s);
//     friend ::us_socket_t *on_timeout(::us_socket_t *s);
//     friend ::us_socket_t *on_connect_error(::us_socket_t *s, int code);
//     friend void on_wakeup(::us_loop_t *loop);
// };

// } // namespace kairo::ip