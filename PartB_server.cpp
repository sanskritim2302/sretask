#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

// Predefined collection
map<string, map<string, int>> collection = {
    {"SetA", {{"One", 1}, {"Two", 2}}},
    {"SetB", {{"Three", 3}, {"Four", 4}}},
    {"SetC", {{"Five", 5}, {"Six", 6}}},
    {"SetD", {{"Seven", 7}, {"Eight", 8}}},
    {"SetE", {{"Nine", 9}, {"Ten", 10}}}
};

// XOR Encryption/Decryption
string xorEncryptDecrypt(const string &input, const string &key) {
    string output = input;
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] ^ key[i % key.size()];
    }
    return output;
}

string SECRET_KEY = "MySecretKey"; // user configurable

void handle_client(int client_socket, sockaddr_in client_addr) {
    char buffer[1024];
    int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        close(client_socket);
        return;
    }

    buffer[bytes] = '\0';
    string encrypted(buffer);
    string request = xorEncryptDecrypt(encrypted, SECRET_KEY);

    cout << "[REQUEST] From " << inet_ntoa(client_addr.sin_addr) 
         << ": (Decrypted) " << request << endl;

    string response;
    size_t dash = request.find("-");
    if (dash != string::npos) {
        string set_name = request.substr(0, dash);
        string key = request.substr(dash + 1);

        if (collection.count(set_name) && collection[set_name].count(key)) {
            int count = collection[set_name][key];
            for (int i = 0; i < count; i++) {
                time_t now = time(0);
                tm *ltm = localtime(&now);
                char time_str[64];
                strftime(time_str, sizeof(time_str), "%d-%m-%Y %H:%M:%S", ltm);

                string msg = string(time_str) + "\n";
                string encrypted_msg = xorEncryptDecrypt(msg, SECRET_KEY);
                send(client_socket, encrypted_msg.c_str(), encrypted_msg.size(), 0);
                this_thread::sleep_for(chrono::seconds(1));
            }
        } else {
            response = "EMPTY\n";
            string encrypted_msg = xorEncryptDecrypt(response, SECRET_KEY);
            send(client_socket, encrypted_msg.c_str(), encrypted_msg.size(), 0);
        }
    } else {
        response = "EMPTY\n";
        string encrypted_msg = xorEncryptDecrypt(response, SECRET_KEY);
        send(client_socket, encrypted_msg.c_str(), encrypted_msg.size(), 0);
    }

    close(client_socket);
    cout << "[DISCONNECTED] " << inet_ntoa(client_addr.sin_addr) << endl;
}

int main() {
    cout << "Enter encryption key: ";
    getline(cin, SECRET_KEY);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "Socket creation failed\n";
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Bind failed\n";
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        cerr << "Listen failed\n";
        return -1;
    }

    cout << "[SERVER STARTED] Listening on port 8080...\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            cerr << "Accept failed\n";
            continue;
        }

        thread t(handle_client, client_socket, client_addr);
        t.detach();
    }

    close(server_fd);
    return 0;
}
