// #include "kairo/ip/WebsocketServer.hpp"
// #include <atomic>
// #include <cstdint>
// #include <mutex>
// #include <shared_mutex>
// #include <stdexcept>
// #include <string>

// namespace kairo::ip
// {

// WebsocketServer::WebsocketServer(
//     int port,
//     std::function<void(const std::string &, const std::string &)>
//         on_message_callback,
//     std::function<void(const std::string &, const std::string &)>
//         on_client_connected_callback,
//     std::function<void(const std::string &)> on_client_disconnected_callback)
//     : port(port), on_message_callback(std::move(on_message_callback)),
//       on_client_connected_callback(std::move(on_client_connected_callback)),
//       on_client_disconnected_callback(
//           std::move(on_client_disconnected_callback))
// {
// }

// WebsocketServer::~WebsocketServer()
// {
//     stop();
// }

// void WebsocketServer::start()
// {
//     if (thread.joinable())
//     {
//         spdlog::warn("WebSocket server is already running.");
//         return;
//     }

//     thread = std::thread(&WebsocketServer::run_loop, this);
//     spdlog::info("WebSocket server starting on port {}...", port);
// }

// void WebsocketServer::stop()
// {
//     spdlog::info("Stopping WebSocket server...");
//     if (thread.joinable())
//     {
//         // Signal loop to stop
//         if (loop)
//         {
//             loop->defer(
//                 [this]()
//                 {
//                     // Close listen socket
//                     if (listen_token)
//                     {
//                         us_listen_socket_close(0, listen_token);
//                         listen_token = nullptr;
//                     }

//                     std::unique_lock<std::shared_mutex> lock(clients_mutex);
//                     for (auto &[id, ws] : clients)
//                     {
//                         ws->close();
//                     }
//                     clients.clear();
//                 });
//         }
//         thread.join();
//         spdlog::info("WebSocket server stopped");
//     }
// }

// void WebsocketServer::send_message_by_ws_id(uint64_t client_id,
//                                             const std::string &message)
// {
//     spdlog::info("Sending message to client ID {}...", client_id);
//     if (!loop)
//     {
//         spdlog::warn("Loop not ready, message to client ID {} not sent.",
//                      client_id);
//         return;
//     }

//     // uWS socket ops must run on the loop thread
//     loop->defer(
//         [this, client_id, message]()
//         {
//             std::shared_lock<std::shared_mutex> lock(clients_mutex);
//             auto it = clients.find(client_id);
//             if (it == clients.end())
//             {
//                 spdlog::warn("Client ID {} not found, message not sent.",
//                              client_id);
//                 return;
//             }
//             it->second->send(message, ::uWS::OpCode::TEXT);
//             spdlog::debug("Sent message to client ID {}, content: {}",
//                           client_id, message);
//         });
// }

// void WebsocketServer::send_message_by_uuid(const std::string &client_uuid,
//                                            std::string message)
// {
//     spdlog::info("Sending message to client UUID {}...", client_uuid);
//     uint64_t client_id;
//     {

//         std::shared_lock<std::shared_mutex> lock(client_uuid_wsid_map_mutex);
//         try
//         {
//             client_id = client_uuid_wsid_map.value(client_uuid);
//         }
//         catch (std::out_of_range &e)
//         {
//             client_id = 0;
//             spdlog::warn("Client UUID {} not found in map.", client_uuid);
//         }
//     }
//     if (client_id != 0)
//     {
//         send_message_by_ws_id(client_id, message);
//     }
//     else
//     {
//         spdlog::warn("Client UUID {} not found, message not sent.",
//                      client_uuid);
//     }
// }

// void test(uWS::HttpResponse<false> *res, uWS::HttpRequest *req,
//           us_socket_context_t *context)
// {
// }

// void WebsocketServer::run_loop()
// {
//     std::atomic_uint64_t next_id{1}; // remove static

//     uWS::App app;
//     loop = ::uWS::Loop::get();

//     app.ws<PerSocketData>(
//            "/*",
//            {.upgrade = [this](auto *res, auto *req, auto *context)
//             { handle_upgrade(res, req, context); },
//             .open =
//                 [this, &next_id](auto *ws)
//             {
//                 auto uuid = handle_open(ws, next_id);
//                 std::string client_ip =
//                     std::string(ws->getRemoteAddressAsText());

//                 if (helpers::shared::is_ipv4_mapped_ipv6(client_ip))
//                 {
//                     client_ip =
//                         helpers::shared::map_ipv4_mapped_to_ipv4(client_ip);
//                 }

//                 spdlog::info("Client connected from IP: {}", client_ip);
//                 // advertise server's public key to the newly connected
//                 client std::string client_uuid;
//                 {
//                     std::shared_lock<std::shared_mutex> lock(
//                         this->client_uuid_wsid_map_mutex);
//                     try
//                     {
//                         client_uuid = this->client_uuid_wsid_map.key(
//                             ws->getUserData()->id);
//                     }
//                     catch (...)
//                     {
//                     }
//                 }
//                 if (client_uuid.empty())
//                 {
//                     spdlog::error("No client UUID found for newly connected "
//                                   "client ID: {}",
//                                   ws->getUserData()->id);
//                     return;
//                 }

//                 std::thread(
//                     [this, client_uuid, client_ip]()
//                     {
//                         std::this_thread::sleep_for(
//                             std::chrono::milliseconds(100));
//                         //
//                         iwebsocket_delegate.ws_on_client_connected(client_uuid);
//                         on_client_connected_callback(client_uuid, client_ip);
//                     })
//                     .detach();
//             },
//             .message = [this](auto *ws, std::string_view message,
//                               ::uWS::OpCode op_code)
//             { handle_message(ws, message, op_code); },
//             .close = [this](auto *ws, int code, std::string_view msg)
//             { handle_close(ws, code, msg); }})
//         .listen(port,
//                 [this](auto *token)
//                 {
//                     if (token)
//                     {
//                         this->listen_token = token;
//                         spdlog::info("Listening on port {}", port);
//                     }
//                     else
//                     {
//                         spdlog::error("Failed to listen on port {}", port);
//                     }
//                 })
//         .run();
// }

// void WebsocketServer::handle_upgrade(auto *res, auto *req, auto *context)
// {
//     PerSocketData initial{};
//     std::string_view id_param = req->getQuery("id");
//     if (!id_param.empty())
//     {
//         uint64_t id = 0;
//         for (char c : id_param)
//         {
//             if (c < '0' || c > '9')
//             {
//                 id = 0;
//                 break;
//             }
//             id = id * 10 + (c - '0');
//         }
//         if (id != 0)
//         {
//             initial.id = id;
//         }
//     }
//     res->template upgrade<PerSocketData>(
//         std::move(initial), req->getHeader("sec-websocket-key"),
//         req->getHeader("sec-websocket-protocol"),
//         req->getHeader("sec-websocket-extensions"), context);
// }

// std::string WebsocketServer::handle_open(auto *ws,
//                                          std::atomic_uint64_t &next_id)
// {
//     auto *ud = ws->getUserData();
//     spdlog::info("New client connection opened with ID {}.", ud->id);
//     if (ud->id == 0)
//     {
//         ud->id = next_id.fetch_add(1, std::memory_order_relaxed);
//     }

//     {
//         std::unique_lock<std::shared_mutex> lock(clients_mutex);
//         clients[ud->id] = ws;
//     }
//     std::string new_client_uuid =
//         libchat::shared::helpers::shared::generate_uuid();
//     {
//         std::unique_lock<std::shared_mutex> lock(client_uuid_wsid_map_mutex);
//         client_uuid_wsid_map.insert(new_client_uuid, ud->id);
//     }
//     // {
//     //     spdlog::info("Getting RSA private key for new client UUID: {}",
//     //     new_client_uuid); std::unique_lock<std::shared_mutex>
//     //     lock(client_uuid_rsa_key_map_mutex);
//     //     client_uuid_rsa_key_map.emplace(new_client_uuid,
//     //     *shared::helpers::rsa::get_ready_rsa_private_key());
//     // }
//     spdlog::info("New Client #{} connected, assigned UUID: {}", ud->id,
//                  new_client_uuid);
//     return new_client_uuid;
// }

// void WebsocketServer::handle_message(auto *ws, std::string_view message_buf,
//                                      ::uWS::OpCode op_code)
// {
//     std::string message_c = std::string(message_buf);
//     auto *ud = ws->getUserData();
//     spdlog::info("Received message from client #{}: len={}", ud->id,
//                  message_c.length());
//     using namespace libchat::shared::enums;

//     std::thread(
//         [this, message_c, ud]()
//         {
//             std::string client_uuid;
//             {
//                 std::shared_lock<std::shared_mutex> lock(
//                     client_uuid_wsid_map_mutex);
//                 try
//                 {
//                     client_uuid = client_uuid_wsid_map.key(ud->id);
//                 }
//                 catch (...)
//                 {
//                 }
//             }
//             if (client_uuid.empty())
//             {
//                 spdlog::error("No client UUID found for client ID: {}, cannot
//                 "
//                               "process message.",
//                               ud->id);
//                 return;
//             }
//             // iwebsocket_delegate.ws_on_message(client_uuid,
//             // std::string(message_c));
//             spdlog::info("WebsocketServer received message from UUID: {}",
//                          client_uuid);
//             on_message_callback(client_uuid, std::string(message_c));
//         })
//         .detach();
// }

// void WebsocketServer::handle_close(auto *ws, int code, std::string_view msg)
// {
//     auto *ud = ws->getUserData();
//     {
//         std::unique_lock<std::shared_mutex> lock(clients_mutex);
//         clients.erase(ud->id);
//     }
//     std::string client_uuid;
//     {
//         std::unique_lock<std::shared_mutex> lock(client_uuid_wsid_map_mutex);
//         client_uuid = client_uuid_wsid_map.key(ud->id);
//         client_uuid_wsid_map.erase_by_value(ud->id);
//     }

//     spdlog::info("Client #{} disconnected (UUID: {}), code: {}, msg: {}",
//                  ud->id, client_uuid, code, msg);
//     on_client_disconnected_callback(client_uuid);
// }

// bool WebsocketServer::is_connected_uuid(const std::string &client_uuid)
// {
//     std::shared_lock<std::shared_mutex> lock(client_uuid_wsid_map_mutex);
//     return client_uuid_wsid_map.contains_key(client_uuid);
// }

// void WebsocketServer::disconnect_client_by_uuid(const std::string
// &client_uuid)
// {
//     if (!loop)
//     {
//         spdlog::warn("Loop not ready, cannot disconnect client UUID {}.",
//                      client_uuid);
//         return;
//     }

//     uint64_t client_id;
//     {
//         std::shared_lock<std::shared_mutex> lock(
//             this->client_uuid_wsid_map_mutex);
//         try
//         {
//             client_id = this->client_uuid_wsid_map.value(client_uuid);
//         }
//         catch (...)
//         {
//             client_id = 0;
//             spdlog::warn("Client UUID {} not found in map.", client_uuid);
//         }
//     }

//     if (client_id == 0)
//     {
//         spdlog::warn("Client UUID {} not found, cannot disconnect.",
//                      client_uuid);
//         return;
//     }
//     // uWS socket ops must run on the loop thread
//     loop->defer(
//         [this, client_id, client_uuid]()
//         {
//             std::shared_lock<std::shared_mutex> lock(this->clients_mutex);
//             auto it = this->clients.find(client_id);
//             if (it != this->clients.end())
//             {
//                 it->second->close();
//                 spdlog::info("Disconnected client UUID: {}", client_uuid);
//             }
//             else
//             {
//                 spdlog::warn(
//                     "Client ID {} not found for UUID {}, cannot disconnect.",
//                     client_id, client_uuid);
//             }
//         });
// }

// } // namespace kairo::ip
