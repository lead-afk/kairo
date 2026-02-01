// #include "kairo/ip/WebsocketClient.hpp"
// #include <cstdlib>
// #include <cstring>
// #include <ctime>
// #include <libusockets.h>
// #include <string>
// #include <sys/types.h>
// #include <thread>

// namespace kairo::ip

// {

// struct ClientSocketData
// {
//     WebsocketClient *client;
//     bool handshake_complete;
//     std::string *buffer;
// };

// // Callbacks
// static ::us_socket_t *on_open(::us_socket_t *s, int is_client, char *ip,
//                               int ip_length)
// {
//     struct ClientSocketData *data =
//         (struct ClientSocketData *)us_socket_ext(0, s);
//     // Do NOT reset data->client here, it is set in run_loop
//     data->handshake_complete = false;
//     data->buffer = new std::string();

//     if (data->client)
//     {
//         data->client->set_socket(s);
//     }

//     // Send Handshake
//     std::string handshake = "GET / HTTP/1.1\r\n"
//                             "Host: localhost\r\n"
//                             "Upgrade: websocket\r\n"
//                             "Connection: Upgrade\r\n"
//                             "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
//                             "Sec-WebSocket-Version: 13\r\n"
//                             "\r\n";

//     us_socket_write(0, s, handshake.c_str(), handshake.length(), 0);
//     return s;
// }

// void WebsocketClient::heartbeat_loop()
// {
//     // Drain any stale permits left from previous stop calls before the
//     thread
//     // existed
//     while (heartbeat_semaphore.try_acquire())
//         ;
//     std::unique_lock<std::mutex> lock(heartbeat_mutex);
//     heartbeat_running = true;
//     while (!heartbeat_stop && connected && !should_stop)
//     {
//         if (heartbeat_semaphore.try_acquire_for(std::chrono::seconds(30)))
//         {
//             break;
//         }

//         lock.unlock();
//         std::string heartbeat_message = std::to_string(
//             static_cast<int>(shared::enums::Message_Type::HEARTBEAT));
//         send_message(heartbeat_message);
//         lock.lock();
//     }
//     heartbeat_running = false;
// }

// static ::us_socket_t *on_data(::us_socket_t *s, char *data, int length)
// {
//     struct ClientSocketData *socketData =
//         (struct ClientSocketData *)us_socket_ext(0, s);

//     if (!socketData->handshake_complete)
//     {
//         std::string response(data, length);
//         if (response.find("101 Switching Protocols") != std::string::npos)
//         {
//             socketData->handshake_complete = true;
//             if (socketData->client)
//             {
//                              socketData->client->get_uri());
//                              socketData->client->set_connected(true);
//                              socketData->client->process_outgoing_queue();
//             }
//         }
//         return s;
//     }

//     // Simple frame parsing (Server -> Client is NOT masked)
//     // Byte 0: FIN(1) RSV(3) Opcode(4)
//     // Byte 1: MASK(1) PayloadLen(7)

//     // If we have a partial message buffer, append new data to it
//     if (socketData->buffer && !socketData->buffer->empty())
//     {
//         socketData->buffer->append(data, length);
//         data = socketData->buffer->data();
//         length = socketData->buffer->size();
//     }

//     int offset = 0;
//     while (offset < length)
//     {
//         if (length - offset < 2)
//             break; // Need at least header

//         unsigned char b0 = data[offset];
//         unsigned char b1 = data[offset + 1];

//         // int opcode = b0 & 0x0F;
//         // bool fin = (b0 >> 7) & 1;

//         uint64_t payload_len = b1 & 0x7F;
//         int header_len = 2;

//         if (payload_len == 126)
//         {
//             if (length - offset < 4)
//                 break; // Need extended header
//             // Next 2 bytes are length (Big Endian)
//             unsigned char l1 = data[offset + 2];
//             unsigned char l2 = data[offset + 3];
//             payload_len = (l1 << 8) | l2;
//             header_len += 2;
//         }
//         else if (payload_len == 127)
//         {
//             if (length - offset < 10)
//                 break; // Need extended header
//             // Next 8 bytes are length (Big Endian)
//             // For simplicity, we only read the lower 32 bits here, assuming
//             // messages aren't > 4GB Real implementation should handle full
//             // 64-bit length
//             unsigned char l5 = data[offset + 6];
//             unsigned char l6 = data[offset + 7];
//             unsigned char l7 = data[offset + 8];
//             unsigned char l8 = data[offset + 9];
//             payload_len = ((uint64_t)l5 << 24) | ((uint64_t)l6 << 16) |
//                           ((uint64_t)l7 << 8) | l8;
//             header_len += 8;
//         }

//         if (offset + header_len + payload_len > length)
//         {
//             // Incomplete frame, should buffer.
//             break;
//         }

//         std::string message(data + offset + header_len, payload_len);
//         if (socketData->client)
//         {
//             socketData->client->on_message(message);
//         }

//         offset += header_len + payload_len;
//     }

//     // If we processed everything, clear buffer.
//     // If we have leftover data (incomplete frame), keep it in buffer.
//     if (offset == length)
//     {
//         if (socketData->buffer)
//             socketData->buffer->clear();
//     }
//     else
//     {
//         // If we were using the raw pointer 'data' but it wasn't from buffer,
//         we
//         // need to init buffer
//         if (socketData->buffer)
//         {
//             if (socketData->buffer->empty())
//             {
//                 socketData->buffer->assign(data + offset, length - offset);
//             }
//             else
//             {
//                 // We were already using buffer, just remove processed part
//                 socketData->buffer->erase(0, offset);
//             }
//         }
//     }

//     return s;
// }

// static ::us_socket_t *on_close(::us_socket_t *s, int code, void *reason)
// {
//     struct ClientSocketData *data =
//         (struct ClientSocketData *)us_socket_ext(0, s);
//     if (data->buffer)
//     {
//         delete data->buffer;
//         data->buffer = nullptr;
//     }
//     if (data->client)
//     {
//         data->client->set_connected(false);
//         data->client->set_socket(nullptr);
//     }
//     return s;
// }

// static ::us_socket_t *on_writable(::us_socket_t *s)
// {
//     return s;
// }

// static ::us_socket_t *on_timeout(::us_socket_t *s)
// {
//     struct ClientSocketData *data =
//         (struct ClientSocketData *)us_socket_ext(0, s);

//     // If we time out before the WebSocket handshake completes, treat this as
//     a
//     // failed connect attempt.
//     if (data && !data->handshake_complete)
//     {
//         if (data->client)
//         {
//             data->client->set_connected(false);
//             data->client->set_socket(nullptr);
//         }
//         return us_socket_close_connecting(0, s);
//     }

//     return s;
// }

// // Called when a connecting socket fails at a late stage (e.g. ECONNREFUSED).
// static ::us_socket_t *on_connect_error(::us_socket_t *s, int code)
// {
//     struct ClientSocketData *data =
//         (struct ClientSocketData *)us_socket_ext(0, s);
//     // ext memory may contain garbage unless we initialize it, so be
//     defensive. if (data)
//     {
//         data->handshake_complete = false;
//         if (data->buffer)
//         {
//             delete data->buffer;
//             data->buffer = nullptr;
//         }
//         if (data->client)
//         {
//             data->client->set_connected(false);
//             data->client->set_socket(nullptr);
//         }
//     }

//     // This is a connecting socket; close it without triggering on_close.
//     return us_socket_close_connecting(0, s);
// }

// static ::us_socket_t *on_end(::us_socket_t *s)
// {
//     return s;
// }

// static void on_wakeup(::us_loop_t *loop)
// {
//     struct LoopData
//     {
//         WebsocketClient *client;
//     };

//     struct LoopData *loopData = (struct LoopData *)us_loop_ext(loop);
//     if (loopData && loopData->client)
//     {
//         loopData->client->process_outgoing_queue();
//     }
// }

// WebsocketClient::WebsocketClient(
//     const std::string &uri,
//     std::function<void(const std::string)> on_message_callback,
//     std::function<void()> on_connect_callback,
//     std::function<void()> on_disconnect_callback)
//     : uri(uri), on_message_callback(on_message_callback),
//       on_connect_callback(on_connect_callback),
//       on_disconnect_callback(on_disconnect_callback)
// {
//     static int instance_count = 0;
//     instance_count++;
//     set_connection_retry_count(
//         libchat::shared::defaults::SHARED_WEBSOCKET_RETRY_COUNT);
// }

// WebsocketClient::~WebsocketClient()
// {
//     disconnect();
// }

// void WebsocketClient::connect()
// {

//     if (client_thread.joinable())
//     {
//         if (!main_loop_ready_to_join)
//         {
//             return;
//         }
//         else
//         {
//             client_thread.join();
//         }
//     }
//     std::srand(std::time(nullptr));
//     should_stop = false;
//     client_thread = std::thread(&WebsocketClient::run_loop, this);
// }

// void WebsocketClient::disconnect()
// {
//     should_stop = true;
//     stop_heartbeat();
//     heartbeat_semaphore.release();
//     if (loop)
//     {
//         us_wakeup_loop((::us_loop_t *)loop);
//     }
//     if (client_thread.joinable())
//     {
//         client_thread.join();
//     }
// }

// void WebsocketClient::send_message(const std::string &message)
// {
//     {
//         std::lock_guard<std::mutex> lock(outgoing_mutex);
//         outgoing_queue.push_back(message);
//     }
//     if (loop)
//     {
//         us_wakeup_loop((::us_loop_t *)loop);
//     }
// }

// void WebsocketClient::set_connected(bool state)
// {
//     bool previous = connected.exchange(state);

//     if (state && !previous)
//     {
//         start_heartbeat();
//         on_connect();
//     }
//     else if (!state && previous)
//     {
//         stop_heartbeat();
//         on_disconnect();
//     }
// }

// void WebsocketClient::on_connect() noexcept
// {
//     if (!on_connect_callback)
//     {
//         return;
//     }

//     try
//     {
//         on_connect_callback();
//     }
//     catch (const std::exception &e)
//     {
//     }
//     catch (...)
//     {
//     }
// }

// void WebsocketClient::on_disconnect() noexcept
// {
//     if (!on_disconnect_callback)
//     {
//         return;
//     }

//     try
//     {
//         on_disconnect_callback();
//     }
//     catch (const std::exception &e)
//     {
//     }
//     catch (...)
//     {
//     }
// }

// void WebsocketClient::set_socket(void *s)
// {
//     socket = s;
// }

// void WebsocketClient::process_outgoing_queue()
// {
//     if (should_stop)
//     {
//         if (socket)
//         {
//             us_socket_close(0, (::us_socket_t *)socket, 0, nullptr);
//             socket = nullptr;
//         }
//         return;
//     }

//     if (!socket || !connected)
//         return;

//     std::deque<std::string> messages;
//     {
//         std::lock_guard<std::mutex> lock(outgoing_mutex);
//         messages.swap(outgoing_queue);
//     }

//     for (const auto &msg : messages)
//     {
//         // Frame the message
//         // FIN=1, Opcode=2 (Binary) -> 0x82
//         // Mask=1, Len=msg.size()
//         std::vector<uint8_t> frame;
//         frame.push_back(0x82);

//         size_t len = msg.size();
//         if (len <= 125)
//         {
//             frame.push_back(0x80 | (uint8_t)len);
//         }
//         else if (len <= 65535)
//         {
//             frame.push_back(0x80 | 126);
//             frame.push_back((len >> 8) & 0xFF);
//             frame.push_back(len & 0xFF);
//         }
//         else
//         {
//             frame.push_back(0x80 | 127);
//             // 64-bit length
//             for (int i = 7; i >= 0; i--)
//             {
//                 frame.push_back((len >> (i * 8)) & 0xFF);
//             }
//         }

//         // Masking key (4 bytes) - using random mask
//         uint8_t mask[4];
//         for (int i = 0; i < 4; i++)
//         {
//             mask[i] = std::rand() % 256;
//         }
//         frame.push_back(mask[0]);
//         frame.push_back(mask[1]);
//         frame.push_back(mask[2]);
//         frame.push_back(mask[3]);

//         // Payload
//         for (size_t i = 0; i < len; i++)
//         {
//             frame.push_back(msg[i] ^ mask[i % 4]);
//         }

//         // std::cout << "Sending frame: len=" << len << " frame_size=" <<
//         // frame.size() << std::endl; std::cout << "Header: " << std::hex;
//         // for(size_t i=0; i<std::min(frame.size(), (size_t)16); ++i) {
//         //     std::cout << (int)frame[i] << " ";
//         // }
//         // std::cout << std::dec << std::endl;
//         us_socket_write(0, (::us_socket_t *)socket, (char *)frame.data(),
//                         frame.size(), 0);
//     }
// }

// void WebsocketClient::on_message(const std::string message)
// {
//     std::thread(
//         [this, message]()
//         {
//             if (this->on_message_callback)
//             {
//                 this->on_message_callback(message);
//             }
//         })
//         .detach();
// }

// struct LoopData
// {
//     WebsocketClient *client;
// };

// void WebsocketClient::run_loop()
// {
//     {
//         std::lock_guard<std::mutex> lock(main_loop_ready_to_join_mutex);
//         main_loop_ready_to_join = false;
//     }

//     this->loop = us_create_loop(
//         0, on_wakeup,             // wakeup
//         [](::us_loop_t *loop) {}, // pre
//         [](::us_loop_t *loop) {}, // post
//         sizeof(struct LoopData));

//     // Store this in loop ext
//     struct LoopData *loopData =
//         (struct LoopData *)us_loop_ext((struct us_loop_t *)this->loop);
//     loopData->client = this;

//     struct us_socket_context_options_t options = {};
//     struct us_socket_context_t *context =
//         us_create_socket_context(0, (struct us_loop_t *)this->loop,
//                                  sizeof(struct ClientSocketData), options);

//     us_socket_context_on_open(0, context, on_open);
//     us_socket_context_on_data(0, context, on_data);
//     us_socket_context_on_close(0, context, on_close);
//     us_socket_context_on_writable(0, context, on_writable);
//     us_socket_context_on_timeout(0, context, on_timeout);
//     us_socket_context_on_connect_error(0, context, on_connect_error);
//     us_socket_context_on_end(0, context, on_end);

//     std::string host;
//     int port;
//     parse_uri(uri, host, port);

//     int retries = defaults::SHARED_WEBSOCKET_RETRY_COUNT;
//     {
//         std::shared_lock<std::shared_mutex>
//         lock(connection_retry_count_mutex); retries = connection_retry_count;
//     }

//     while (retries > 0 && !should_stop)
//     {
//         // NOTE: us_socket_context_connect can return a socket even if the
//         // connection will fail asynchronously. In that case,
//         on_connect_error
//         // will fire once the loop runs.
//         struct us_socket_t *s =
//             us_socket_context_connect(0, context, host.c_str(), port,
//             nullptr,
//                                       0, sizeof(struct ClientSocketData));

//         // Avoid long OS-level connect timeouts for unreachable hosts.
//         // This timeout is in seconds.
//         us_socket_timeout(0, s,
//         defaults::SHARED_WEBSOCKET_CONNECT_TIMEOUT_S);

//         // if (!s)
//         // {
//         //     spdlog::warn("Failed to start connect to {}:{}. Retrying in 2
//         //     seconds... ({} attempts left)", host, port,
//         //                  retries);
//         //     std::this_thread::sleep_for(std::chrono::seconds(2));
//         //     --retries;
//         //     continue;
//         // }

//         // Initialize ext storage. libusockets does not run C++ constructors
//         // here, so ensure pointers are null.
//         struct ClientSocketData *data =
//             (struct ClientSocketData *)us_socket_ext(0, s);
//         std::memset(data, 0, sizeof(ClientSocketData));
//         data->client = this;
//         data->handshake_complete = false;
//         data->buffer = nullptr;

//         // Drive the loop. If the connection fails, on_connect_error will
//         close
//         // the connecting socket and the loop may fall through (no active
//         // polls), allowing us to retry.
//         us_loop_run((struct us_loop_t *)this->loop);

//         if (should_stop)
//         {
//             break;
//         }

//         // If we returned here without being asked to stop, treat it as a
//         failed
//         // attempt and retry.
//         const auto retry_interval =
//             libchat::shared::defaults::SHARED_WEBSOCKET_RETRY_INTERVAL_MS;
//         spdlog::warn("Disconnected or failed to connect to {}:{}. Retrying in
//         "
//                      "{} ms... ({} attempts left)",
//                      host, port, retry_interval.count(), retries - 1);
//         std::this_thread::sleep_for(retry_interval);
//         --retries;
//     }

//     if (!should_stop && retries == 0)
//     {
//         spdlog::error("Failed to connect after multiple attempts. Client "
//                       "websocket thread exiting.");
//     }

//     us_socket_context_free(0, context);
//     us_loop_free((struct us_loop_t *)this->loop);
//     this->loop = nullptr;
//     {
//         std::lock_guard<std::mutex> lock(main_loop_ready_to_join_mutex);
//         main_loop_ready_to_join = true;
//     }
// }

// void WebsocketClient::start_heartbeat()
// {
//     stop_heartbeat();
//     {
//         std::lock_guard<std::mutex> lock(heartbeat_mutex);
//         heartbeat_stop = false;
//     }
//     heartbeat_thread = std::thread(&WebsocketClient::heartbeat_loop, this);
// }

// void WebsocketClient::stop_heartbeat()
// {
//     bool should_release = false;
//     {
//         std::lock_guard<std::mutex> lock(heartbeat_mutex);
//         heartbeat_stop = true;
//         should_release = heartbeat_running;
//     }
//     if (should_release)
//     {
//         heartbeat_semaphore.release();
//     }
//     if (heartbeat_thread.joinable())
//     {
//         heartbeat_thread.join();
//     }
//     {
//         std::lock_guard<std::mutex> lock(heartbeat_mutex);
//         heartbeat_stop = false;
//     }
// }

// bool WebsocketClient::is_connected() const
// {
//     return connected;
// }

// } // namespace kairo::ip
