
// chat_client.cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#include <thread>
#include <atomic>
#include <locale>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Типы пакетов (такие же как на сервере)
enum PacketType {
    PACKET_NICKNAME_REQUEST = 1,
    PACKET_NICKNAME_ACCEPTED,
    PACKET_NICKNAME_REJECTED,
    PACKET_MESSAGE,
    PACKET_PRIVATE_MESSAGE,
    PACKET_USER_LIST,
    PACKET_USER_JOINED,
    PACKET_USER_LEFT,
    PACKET_SERVER_MESSAGE,
    PACKET_DISCONNECT
};

atomic<bool> isRunning(true);
string nickname;

// Функция для установки русской локализации
void setRussianLocale() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");
}

// Функция для отправки пакета серверу
bool sendPacket(SOCKET serverSocket, PacketType type, const string& data) {
    int packetType = type;
    int dataLength = (int)data.length();

    if (send(serverSocket, (char*)&packetType, sizeof(int), 0) == SOCKET_ERROR) {
        return false;
    }

    if (send(serverSocket, (char*)&dataLength, sizeof(int), 0) == SOCKET_ERROR) {
        return false;
    }

    if (dataLength > 0) {
        if (send(serverSocket, data.c_str(), dataLength, 0) == SOCKET_ERROR) {
            return false;
        }
    }

    return true;
}

// Функция для получения пакета от сервера
bool receivePacket(SOCKET serverSocket, PacketType& type, string& data) {
    int packetType;
    int dataLength;

    if (recv(serverSocket, (char*)&packetType, sizeof(int), 0) <= 0) {
        return false;
    }
    type = static_cast<PacketType>(packetType);

    if (recv(serverSocket, (char*)&dataLength, sizeof(int), 0) <= 0) {
        return false;
    }

    if (dataLength > 0) {
        char* buffer = new char[dataLength + 1];
        int bytesReceived = recv(serverSocket, buffer, dataLength, 0);
        if (bytesReceived <= 0) {
            delete[] buffer;
            return false;
        }
        buffer[bytesReceived] = '\0';
        data = string(buffer, bytesReceived);
        delete[] buffer;
    }
    else {
        data = "";
    }

    return true;
}

// Функция приема сообщений от сервера
void receiveMessages(SOCKET serverSocket) {
    while (isRunning) {
        PacketType type;
        string data;

        if (!receivePacket(serverSocket, type, data)) {
            cout << endl << "[ОШИБКА] Соединение с сервером потеряно!" << endl;
            isRunning = false;
            break;
        }

        switch (type) {
        case PACKET_NICKNAME_REQUEST:
            cout << endl << data << ": ";
            break;

        case PACKET_NICKNAME_ACCEPTED:
            cout << endl << "[УСПЕХ] " << data << endl;
            cout << endl << "=== ЧАТ НАЧАТ ===" << endl;
            cout << "Команды:" << endl;
            cout << "  @никнейм сообщение - отправить личное сообщение" << endl;
            cout << "  /users - показать пользователей онлайн" << endl;
            cout << "  /exit - выйти из чата" << endl;
            cout << "===================" << endl << endl;
            break;

        case PACKET_NICKNAME_REJECTED:
            cout << endl << "[ОШИБКА] " << data << endl;
            isRunning = false;
            break;

        case PACKET_MESSAGE:
            cout << data << endl;
            break;

        case PACKET_PRIVATE_MESSAGE:
            // Выделяем личные сообщения
            cout << "[ЛИЧНОЕ] " << data << endl;
            break;

        case PACKET_USER_LIST:
            cout << endl << "=== ПОЛЬЗОВАТЕЛИ ОНЛАЙН ===" << endl;
            cout << data << endl;
            break;

        case PACKET_USER_JOINED:
            cout << "[ИНФО] " << data << endl;
            break;

        case PACKET_USER_LEFT:
            cout << "[ИНФО] " << data << endl;
            break;

        case PACKET_SERVER_MESSAGE:
            cout << "[СЕРВЕР] " << data << endl;
            break;

        default:
            break;
        }
    }
}

// Функция для обработки ввода пользователя
void handleUserInput(SOCKET serverSocket) {
    string input;

    while (isRunning) {
        cout << "Вы: ";
        getline(cin, input);

        if (!isRunning) break;

        if (input.empty()) {
            continue;
        }

        // Команда выхода
        if (input == "/exit" || input == "/quit") {
            sendPacket(serverSocket, PACKET_DISCONNECT, "");
            isRunning = false;
            break;
        }

        // Команда для получения списка пользователей
        else if (input == "/users" || input == "/who") {
            sendPacket(serverSocket, PACKET_MESSAGE, "/users");
        }

        // Приватное сообщение (формат: @username message)
        else if (input[0] == '@') {
            size_t spacePos = input.find(' ');
            if (spacePos != string::npos && spacePos > 1) {
                string toUser = input.substr(1, spacePos - 1);
                string message = input.substr(spacePos + 1);

                if (!message.empty()) {
                    sendPacket(serverSocket, PACKET_PRIVATE_MESSAGE, toUser + ":" + message);
                }
                else {
                    cout << "Сообщение не может быть пустым!" << endl;
                }
            }
            else {
                cout << "Формат: @никнейм сообщение" << endl;
            }
        }

        // Публичное сообщение
        else {
            sendPacket(serverSocket, PACKET_MESSAGE, input);
        }
    }
}

int main(int argc, char* argv[]) {
    // Устанавливаем русскую локализацию
    setRussianLocale();

    cout << "=== ЧАТ-КЛИЕНТ ===" << endl;
    cout << "Многопользовательский чат с приватными сообщениями" << endl;
    cout << "Версия 1.0" << endl;
    cout << "==================" << endl << endl;

    string serverIP = "127.0.0.1";
    int serverPort = 5555;

    // Парсинг аргументов командной строки
    if (argc >= 3) {
        serverIP = argv[1];
        serverPort = atoi(argv[2]);
    }
    else if (argc == 2) {
        serverIP = argv[1];
    }

    cout << "Подключаемся к " << serverIP << ":" << serverPort << "..." << endl;

    // Инициализация WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "ОШИБКА: Не удалось инициализировать WinSock!" << endl;
        return 1;
    }

    // Создание сокета
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "ОШИБКА: Не удалось создать сокет!" << endl;
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    // Подключение к серверу
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "ОШИБКА: Не удалось подключиться к серверу!" << endl;
        cerr << "Убедитесь, что сервер запущен на " << serverIP << ":" << serverPort << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "[УСПЕХ] Подключено к серверу!" << endl;

    // Получение запроса на ввод ника
    PacketType type;
    string data;
    if (!receivePacket(clientSocket, type, data)) {
        cerr << "ОШИБКА: Не удалось получить данные от сервера!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    if (type != PACKET_NICKNAME_REQUEST) {
        cerr << "ОШИБКА: Неожиданный ответ от сервера!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Ввод ника
    cout << data << ": ";
    getline(cin, nickname);

    // Отправка ника серверу
    if (!sendPacket(clientSocket, PACKET_NICKNAME_REQUEST, nickname)) {
        cerr << "ОШИБКА: Не удалось отправить никнейм!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Запуск потока для приема сообщений
    thread receiveThread(receiveMessages, clientSocket);

    // Обработка ввода пользователя в основном потоке
        handleUserInput(clientSocket);

    // Ожидание завершения потока приема
    isRunning = false;
    if (receiveThread.joinable()) {
        receiveThread.join();
    }

    // Закрытие соединения
    cout << endl << "Отключаемся от сервера..." << endl;
    closesocket(clientSocket);
    WSACleanup();

    cout << "До свидания!" << endl;
    system("pause");

    return 0;
}