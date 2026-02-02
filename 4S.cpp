
// chat_server.cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <algorithm>
#include <locale>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Структура для хранения информации о клиенте
struct ClientInfo {
    SOCKET socket;
    string nickname;
    string ipAddress;
    bool isConnected;
};

// Типы пакетов
enum PacketType {
    PACKET_NICKNAME_REQUEST = 1,    // Запрос ника при подключении
    PACKET_NICKNAME_ACCEPTED,       // Ник принят
    PACKET_NICKNAME_REJECTED,       // Ник отклонен (уже занят)
    PACKET_MESSAGE,                 // Публичное сообщение
    PACKET_PRIVATE_MESSAGE,         // Приватное сообщение
    PACKET_USER_LIST,               // Список пользователей онлайн
    PACKET_USER_JOINED,             // Новый пользователь присоединился
    PACKET_USER_LEFT,               // Пользователь вышел
    PACKET_SERVER_MESSAGE,          // Сообщение от сервера
    PACKET_DISCONNECT               // Команда отключения
};

// Глобальные переменные
vector<ClientInfo> clients;
mutex clientsMutex;
mutex consoleMutex;

// Функция для установки русской локализации
void setRussianLocale() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");
}

// Функция для вывода сообщений в консоль
void printMessage(const string& message, bool isError = false) {
    lock_guard<mutex> lock(consoleMutex);
    if (isError) {
        cerr << "[ОШИБКА] " << message << endl;
    }
    else {
        cout << "[ИНФО] " << message << endl;
    }
}

// Функция для отправки пакета клиенту
bool sendPacket(SOCKET clientSocket, PacketType type, const string& data) {
    // Формат пакета: [тип пакета (4 байта)][длина данных (4 байта)][данные]
    int packetType = type;
    int dataLength = (int)data.length();

    // Отправляем тип пакета
    if (send(clientSocket, (char*)&packetType, sizeof(int), 0) == SOCKET_ERROR) {
        return false;
    }

    // Отправляем длину данных
    if (send(clientSocket, (char*)&dataLength, sizeof(int), 0) == SOCKET_ERROR) {
        return false;
    }

    // Отправляем данные, если они есть
    if (dataLength > 0) {
        if (send(clientSocket, data.c_str(), dataLength, 0) == SOCKET_ERROR) {
            return false;
        }
    }

    return true;
}

// Функция для получения пакета от клиента
bool receivePacket(SOCKET clientSocket, PacketType& type, string& data) {
    int packetType;
    int dataLength;

    // Получаем тип пакета
    if (recv(clientSocket, (char*)&packetType, sizeof(int), 0) <= 0) {
        return false;
    }
    type = static_cast<PacketType>(packetType);

    // Получаем длину данных
    if (recv(clientSocket, (char*)&dataLength, sizeof(int), 0) <= 0) {
        return false;
    }

    // Получаем данные, если они есть
    if (dataLength > 0) {
        char* buffer = new char[dataLength + 1];
        int bytesReceived = recv(clientSocket, buffer, dataLength, 0);
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

// Функция для проверки уникальности ника
bool isNicknameUnique(const string& nickname) {
    lock_guard<mutex> lock(clientsMutex);
    for (const auto& client : clients) {
        if (client.nickname == nickname && client.isConnected) {
            return false;
        }
    }
    return true;
}

// Функция для отправки сообщения всем клиентам
void broadcastMessage(const string& message, SOCKET excludeSocket = INVALID_SOCKET) {
    lock_guard<mutex> lock(clientsMutex);

    for (const auto& client : clients) {
        if (client.isConnected && client.socket != excludeSocket) {
            sendPacket(client.socket, PACKET_MESSAGE, message);
        }
    }
}

// Функция для отправкиприватногоо сообщения
bool sendPrivateMessage(const string& fromNickname, const string& toNickname, const string& message) {
    lock_guard<mutex> lock(clientsMutex);

    bool recipientFound = false;
    SOCKET recipientSocket = INVALID_SOCKET;

    // Ищем получателя
    for (const auto& client : clients) {
        if (client.nickname == toNickname && client.isConnected) {
            recipientFound = true;
            recipientSocket = client.socket;
            break;
        }
    }

    if (!recipientFound) {
        return false;
    }

    // Формируем сообщение для получателя
    string privateMessage = "[ЛС от " + fromNickname + "]: " + message;
    sendPacket(recipientSocket, PACKET_PRIVATE_MESSAGE, privateMessage);

    // Формируем сообщение для отправителя
    string senderMessage = "[ЛС для " + toNickname + "]: " + message;

    // Находим сокет отправителя
    for (const auto& client : clients) {
        if (client.nickname == fromNickname && client.isConnected) {
            sendPacket(client.socket, PACKET_PRIVATE_MESSAGE, senderMessage);
            break;
        }
    }

    return true;
}

// Функция для получения списка пользователей онлайн
string getOnlineUsersList() {
    lock_guard<mutex> lock(clientsMutex);

    int onlineCount = 0;
    for (const auto& client : clients) {
        if (client.isConnected) onlineCount++;
    }

    string userList = "Пользователи онлайн (" + to_string(onlineCount) + "):\n";
    for (const auto& client : clients) {
        if (client.isConnected) {
            userList += "  * " + client.nickname + " (" + client.ipAddress + ")\n";
        }
    }
    return userList;
}

// Функция обработки клиента
void handleClient(SOCKET clientSocket, string clientIP) {
    string nickname = "";

    try {
        // Запрашиваем ник у клиента
        sendPacket(clientSocket, PACKET_NICKNAME_REQUEST, "Введите ваш никнейм:");

        PacketType packetType;
        string data;

        // Получаем ник от клиента
        if (!receivePacket(clientSocket, packetType, data)) {
            printMessage("Не удалось получить ник от " + clientIP, true);
            closesocket(clientSocket);
            return;
        }

        if (packetType != PACKET_NICKNAME_REQUEST || data.empty()) {
            sendPacket(clientSocket, PACKET_SERVER_MESSAGE, "Неверный формат ника");
            closesocket(clientSocket);
            return;
        }

        nickname = data;

        // Проверяем уникальность ника
        if (!isNicknameUnique(nickname)) {
            sendPacket(clientSocket, PACKET_NICKNAME_REJECTED, "Никнейм уже занят");
            printMessage("Никнейм отклонен: " + nickname + " (" + clientIP + ")");
            closesocket(clientSocket);
            return;
        }

        // Ник принят
        sendPacket(clientSocket, PACKET_NICKNAME_ACCEPTED, "Добро пожаловать в чат, " + nickname + "!");

        // Добавляем клиента в список
        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back({ clientSocket, nickname, clientIP, true });
        }

        printMessage("Пользователь подключился: " + nickname + " (" + clientIP + ")");

        // Отправляем список пользователей новому клиенту
        sendPacket(clientSocket, PACKET_USER_LIST, getOnlineUsersList());

        // Оповещаем всех о новом пользователе
        string joinMessage = "Пользователь '" + nickname + "' присоединился к чату!";
        broadcastMessage(joinMessage, clientSocket);
        sendPacket(clientSocket, PACKET_USER_JOINED, "Вы успешно присоединились к чату!");

        // Основной цикл обработки сообщений
        while (true) {
            if (!receivePacket(clientSocket, packetType, data)) {
                break;
            }

            switch (packetType) {
            case PACKET_MESSAGE: {
                // Публичное сообщение
                if (data == "/users" || data == "/who") {
                    // Запрос списка пользователей
                    sendPacket(clientSocket, PACKET_USER_LIST, getOnlineUsersList());
                }
                else {
                    string message = "[" + nickname + "]: " + data;
                    printMessage("Публичное сообщение от " + nickname + ": " + data);
                    broadcastMessage(message, clientSocket);
                }
                break;
            }

            case PACKET_PRIVATE_MESSAGE: {
                // Приватное сообщение: формат "ник: сообщение"
                size_t colonPos = data.find(':');
                if (colonPos != string::npos && colonPos > 0) {
                    string toNickname = data.substr(0, colonPos);
                    string message = data.substr(colonPos + 1);

                    // Убираем пробелы в начале сообщения
                    while (!message.empty() && message[0] == ' ') {
                        message.erase(0, 1);
                    }

                    if (sendPrivateMessage(nickname, toNickname, message)) {
                        printMessage("Приватное сообщение от " + nickname + " для " + toNickname + ": " + message);
                    }
                    else {
                        string errorMsg = "Пользователь '" + toNickname + "' не найден или не в сети";
                        sendPacket(clientSocket, PACKET_SERVER_MESSAGE, errorMsg);
                    }
                }
                break;
            }

            case PACKET_DISCONNECT: {
                // Клиент хочет отключиться
                throw runtime_error("Клиент запросил отключение");
            }

            default:
                break;
            }
        }
    }
    catch (...) {
        // Исключение - разрыв соединения
    }
    // Удаляем клиента из списка
    string leftNickname = "";
    string leftIP = "";
    {
        lock_guard<mutex> lock(clientsMutex);

        auto it = find_if(clients.begin(), clients.end(),
            [clientSocket](const ClientInfo& ci) { return ci.socket == clientSocket; });
        if (it != clients.end()) {
            leftNickname = it->nickname;
            leftIP = it->ipAddress;
            clients.erase(it);  // Удаляем сразу, не используем флаг isConnected
        }
    }

    // Отправляем уведомления ВНЕ критической секции (во избежание дедлока!)
    if (!leftNickname.empty()) {
        string leaveMessage = "Пользователь '" + leftNickname + "' покинул чат.";
        printMessage("Пользователь отключился: " + leftNickname + " (" + leftIP + ")");
        broadcastMessage(leaveMessage);
    }

    closesocket(clientSocket);
}

// Функция для отображения статистики сервера
void displayServerInfo() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(10));

        lock_guard<mutex> lock(clientsMutex);
        int onlineCount = 0;
        for (const auto& client : clients) {
            if (client.isConnected) onlineCount++;
        }

        printMessage("=== СТАТУС СЕРВЕРА ===");
        printMessage("Пользователей онлайн: " + to_string(onlineCount));
        printMessage("Всего подключений: " + to_string(clients.size()));
        printMessage("=====================\n");
    }
}

int main() {
    // Устанавливаем русскую локализацию
    setRussianLocale();

    cout << "=== ЧАТ-СЕРВЕР ===" << endl;
    cout << "Многопоточный чат-сервер с приватными сообщениями" << endl;
    cout << "Версия 1.0" << endl;
    cout << "==================" << endl << endl;

    // Инициализация WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "ОШИБКА: Не удалось инициализировать WinSock!" << endl;
        return 1;
    }

    // Создание сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "ОШИБКА: Не удалось создать сокет!" << endl;
        WSACleanup();
        return 1;
    }

   
        // Включаем повторное использование порта
        int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // Настройка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5555);  // Порт чата

    // Привязка сокета
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "ОШИБКА: Порт 5555 занят!" << endl;
        cerr << "Попробуйте использовать другой порт" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Начало прослушивания
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "ОШИБКА: Не удалось начать прослушивание!" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "[УСПЕХ] Сервер успешно запущен!" << endl;
    cout << "[АДРЕС] 127.0.0.1" << endl;
    cout << "[ПОРТ] 5555" << endl;
    cout << "[СТАТУС] Ожидание подключений..." << endl << endl;

    // Запуск потока для отображения статистики
    thread infoThread(displayServerInfo);
    infoThread.detach();

    vector<thread> clientThreads;

    // Основной цикл сервера
    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);

        // Принятие подключения
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            printMessage("Не удалось принять подключение!", true);
            continue;
        }

        // Получение IP клиента
        string clientIP = inet_ntoa(clientAddr.sin_addr);

        // Запуск потока для обработки клиента
        clientThreads.emplace_back(handleClient, clientSocket, clientIP);

        // Очистка завершенных потоков
        clientThreads.erase(
            remove_if(clientThreads.begin(), clientThreads.end(),
                [](thread& t) { return !t.joinable(); }),
            clientThreads.end()
        );
    }

    // Закрытие сокета (никогда не выполнится в бесконечном цикле)
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}