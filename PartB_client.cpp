#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

// XOR Encryption/Decryption
string xorEncryptDecrypt(const string &input, const string &key) {
    string output = input;
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] ^ key[i % key.size()];
    }
    return output;
}

string SECRET_KEY = "MySecretKey";

int main() {
    cout << "Enter encryption key: ";
    getline(cin, SECRET_KEY);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Socket creation failed\n";
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        cerr << "Invalid address\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Connection failed\n";
        return -1;
    }

    string message;
    cout << "Enter request (e.g. SetA-Two): ";
    getline(cin, message);

    string encrypted = xorEncryptDecrypt(message, SECRET_KEY);
    send(sock, encrypted.c_str(), encrypted.size(), 0);

    char buffer[1024];
    int bytes;
    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = '\0';
        string encrypted_msg(buffer);
        string decrypted_msg = xorEncryptDecrypt(encrypted_msg, SECRET_KEY);
        cout << decrypted_msg;
    }

    close(sock);
    return 0;
}
