#include "tcp_sponge_socket.hh"
#include "util.hh"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

// void get_URL(const string &host, const string &path) {

//   cout << "get_URL() called" << endl;

//   // You will need to connect to the "http" service on
//   // the computer whose name is in the "host" string,
//   // then request the URL path given in the "path" string.
//   CS144TCPSocket socket{};
//   socket.connect(Address(host, "http" /* service */));
//   // Note: \r\n\r\n should be the end of outstream.
//   string content = "GET " + path + " HTTP/1.1\r\n" +  // GET /hello HTTP/1.1
//                     "Host: " + host + "\r\n" +  // Host: cs144.keithw.org
//                     "Connection: close\r\n\r\n";  // Connection: close
//   socket.write(content);

//   cout << "after write" << endl;

//   socket.shutdown(SHUT_WR);

//   cout << "after shutdown" << endl;

//   // Then you'll need to print out everything the server sends back,
//   // (not just one call to read() -- everything) until you reach
//   // the "eof" (end of file).
//   while (!socket.eof()) {
//     cout << socket.read();
//   }

//   cout << "after read" << endl;

//   cout << flush;
//   socket.wait_until_closed();

//   cout << "after wait close" << endl;
// }

void get_URL(const string &host, const string &path) {
    CS144TCPSocket sock{};
    sock.connect(Address(host, "http"));
    string input("GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
    sock.write(input);
    // cout<<input;
    // If you don’t shut down your outgoing byte stream,
    // the server will wait around for a while for you to send
    // additional requests and won’t end its outgoing byte stream either.
    sock.shutdown(SHUT_WR);
    while (!sock.eof())
        cout << sock.read();
    sock.close();

    // the Linux kernel takes care of waiting forTCP connections to reach “clean shutdown” (
    // and give up their port reservations) even after user processes have exited.
    // because CS144TCP implementation is all in user space,
    // there’s nothing else to keep track of the connection state except using this call waiting until
    // TCPConnection reports active() = false.
    sock.wait_until_closed();
}

int main(int argc, char *argv[]) {
  try {
    if (argc <= 0) {
      abort();  // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if (argc != 3) {
      cerr << "Usage: " << argv[0] << " HOST PATH\n";
      cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host = argv[1];
    const string path = argv[2];

    // Call the student-written function.
    get_URL(host, path);
  } catch (const exception &e) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
