#include "tcp_connection.hh"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;
using namespace std::chrono;

constexpr size_t len = 100 * 1024 * 1024;

void move_segments(TCPConnection &from, TCPConnection &to, vector<TCPSegment> &segments, const bool reorder) {
    while (!from.segments_out().empty()) {
        segments.emplace_back(move(from.segments_out().front()));
        from.segments_out().pop();
    }
    if (reorder) {
        for (auto it = segments.rbegin(); it != segments.rend(); ++it) {
            to.segment_received(move(*it));
        }
    } else {
        for (auto it = segments.begin(); it != segments.end(); ++it) {
            to.segment_received(move(*it));
        }
    }
    segments.clear();
}

void main_loop(const bool reorder) {
    TCPConfig config;
    TCPConnection client{config};
    TCPConnection server{config};

    string string_to_send(len, 'x');
    for (auto &ch : string_to_send) {
        ch = rand();
    }

    Buffer bytes_to_send{string(string_to_send)};
    client.connect();  // add SYN to client's segment_out
    server.end_input_stream();  // close server's write-end(server only reads)

    bool client_closed = false;

    string string_received;
    string_received.reserve(len);

    const auto first_time = high_resolution_clock::now();

    auto loop = [&] {
        // write input into client
        while (bytes_to_send.size() != 0 && client.remaining_outbound_capacity() != 0) {
            const auto want = min(client.remaining_outbound_capacity(), bytes_to_send.size());
            const auto written = client.write(string(bytes_to_send.str().substr(0, want)));  // add bytes into client's segment out
            if (want != written) {
                throw runtime_error("want = " + to_string(want) + ", written = " + to_string(written));
            }
            bytes_to_send.remove_prefix(written);  // remove sent bytes from bytes-to-send
        }

        // All bytes to send has been added to client's segment out.
        if (bytes_to_send.size() == 0 && !client_closed) {
            client.end_input_stream();  // close client's write-end(only leaves its read-end)
            client_closed = true;
        }

        // exchange segments between client and server but in reverse order
        vector<TCPSegment> segments;
        move_segments(client, server, segments, reorder);
        move_segments(server, client, segments, false);

        // read output from server
        const auto available_output = server.inbound_stream().buffer_size();
        if (available_output > 0) {
            string_received.append(server.inbound_stream().read(available_output));
        }

        // time passes
        client.tick(1000);
        server.tick(1000);
    };

    while (!server.inbound_stream().eof()) {
        loop();
    }

    if (string_received != string_to_send) {
        throw runtime_error("strings sent vs. received don't match");
    }

    const auto final_time = high_resolution_clock::now();

    const auto duration = duration_cast<nanoseconds>(final_time - first_time).count();

    const auto gigabits_per_second = len * 8.0 / double(duration);

    cout << fixed << setprecision(2);
    cout << "CPU-limited throughput" << (reorder ? " with reordering: " : "                : ") << gigabits_per_second
         << " Gbit/s\n";

    while (client.active() or server.active()) {
        loop();
    }
}

int main() {
    try {
        main_loop(false);
        main_loop(true);
    } catch (const exception &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
