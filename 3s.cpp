
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#include <ws2tcpip.h>
#include <ctime>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

string getCurrentDateTime() {
    time_t now = time(0);
    tm timeinfo;
    localtime_s(&timeinfo, &now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return string(buffer);
}

string getClientIP(sockaddr_in clientAddr) {
    // Простая версия без inet_ntop (которая может вызывать проблемы)
    char* ipStr = inet_ntoa(clientAddr.sin_addr);
    if (ipStr) {
        return string(ipStr);
    }
    return "unknown";
}

int main() {
    // Устанавливаем русскую кодировку для консоли
    system("chcp 1251 > nul");

    cout << "=== HTTP-СЕРВЕР НА C++ ===" << endl;
    cout << "Веб-сервер с русским интерфейсом и английским сайтом\n" << endl;

    // Инициализация WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "ОШИБКА: Не удалось инициализировать WinSock!" << endl;
        return 1;
    }
    cout << "✓ WinSock инициализирован" << endl;

    // Создание сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "ОШИБКА: Не удалось создать сокет!" << endl;
        cerr << "Код ошибки: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    cout << "✓ Сокет создан" << endl;

    // Включаем повторное использование порта
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        cerr << "Предупреждение: Не удалось установить SO_REUSEADDR" << endl;
        // Продолжаем выполнение, это не критично
    }

    // Настройка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Пробуем разные порты
    int ports[] = { 8080, 8888, 8081, 9090, 8000, 8082 };
    int selectedPort = -1;

    for (int i = 0; i < sizeof(ports) / sizeof(ports[0]); i++) {
        serverAddr.sin_port = htons(ports[i]);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
            selectedPort = ports[i];
            cout << "✓ Используем порт: " << selectedPort << endl;
            break;
        }
        else {
            int error = WSAGetLastError();
            cout << "✗ Порт " << ports[i] << " занят (ошибка " << error << ")" << endl;
        }
    }

    if (selectedPort == -1) {
        cerr << "ОШИБКА: Все порты заняты!" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Начало прослушивания
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "ОШИБКА: Не удалось начать прослушивание!" << endl;
        cerr << "Код ошибки: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "\n========================================" << endl;
    cout << "✅ СЕРВЕР ЗАПУЩЕН!" << endl;
    cout << "📍 Адрес: http://localhost:" << selectedPort << endl;
    cout << "📍 Адрес: http://127.0.0.1:" << selectedPort << endl;
    cout << "📡 Статус: Ожидание подключений..." << endl;
    cout << "========================================\n" << endl;

    // Основной цикл сервера
    int connectionCount = 0;

    while (true) {
        cout << "[ОЖИДАНИЕ] Сервер ожидает подключения..." << endl;

        // Принятие подключения
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            int error = WSAGetLastError();
            cerr << "ОШИБКА: Не удалось принять подключение! Код: " << error << endl;
            continue; // Продолжаем работу, не выходим
        }

        connectionCount++;
        string clientIP = getClientIP(clientAddr);
        string currentTime = getCurrentDateTime();

        
            cout << "[" << currentTime << "] ";
        cout << "🔗 Подключение #" << connectionCount << " от: " << clientIP << endl;

        // Чтение HTTP-запроса
        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        string requestMethod = "GET";
        string requestPath = "/";

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string request(buffer);

            // Парсим HTTP метод и путь
            size_t firstSpace = request.find(' ');
            if (firstSpace != string::npos) {
                requestMethod = request.substr(0, firstSpace);
                size_t secondSpace = request.find(' ', firstSpace + 1);
                if (secondSpace != string::npos) {
                    requestPath = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);
                }
            }

            cout << "   📨 " << requestMethod << " " << requestPath << " (" << bytesReceived << " байт)" << endl;
        }
        else if (bytesReceived == 0) {
            cout << "   📭 Клиент отключился" << endl;
            closesocket(clientSocket);
            continue;
        }
        else {
            cerr << "   ❌ Ошибка чтения запроса: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            continue;
        }

        // Простой HTML на английском
        string html = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>C++ HTTP Server</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        
        .container {
            background: white;
            border-radius: 10px;
            padding: 40px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            max-width: 800px;
            width: 100%;
        }
        
        h1 {
            color: #333;
            border-bottom: 3px solid #667eea;
            padding-bottom: 10px;
            margin-top: 0;
        }
        
        .status {
            background: #4CAF50;
            color: white;
            padding: 10px 20px;
            border-radius: 20px;
            display: inline-block;
            margin-bottom: 20px;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
            margin: 20px 0;
        }
        
        .info-item {
            background: #f5f5f5;
            padding: 15px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        
        .request-info {
            background: #fff8e1;
            border-left: 4px solid #ffb300;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
        }
        
        .footer {
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
            color: #666;
            text-align: center;
        }
        
        .highlight {
            font-weight: bold;
            color: #667eea;
        }
        
        .tech-list {
            list-style: none;
            padding: 0;
        }
        
        .tech-list li {
            padding: 8px 0;
            border-bottom: 1px solid #eee;
        }
        
        .tech-list li:last-child {
            border-bottom: none;
        }
        
        .tech-list li:before {
            content: "✓ ";
            color: #4CAF50;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1> ===   C++ HTTP Server</h1>
        <div class="status"> Server is running</div>
        
        <p>Welcome to the C++ HTTP Server. This server is written in C++ using WinSock API.</p>
        
        <div class="info-grid">
            <div class="info-item">

Егорик, [01.02.2026 13:12]
<strong>Port:</strong> )" + to_string(selectedPort) + R"(
            </div>
            <div class="info-item">
                <strong>Protocol:</strong> HTTP/1.1
            </div>
            <div class="info-item">
                <strong>Language:</strong> C++
            </div>
            <div class="info-item">
                <strong>Connections:</strong> )" + to_string(connectionCount) + R"(
            </div>
        </div>
        
        <div class="request-info">
            <h3>📊 Request Information</h3>
            <p><strong>Client IP:</strong> <span class="highlight">)" + clientIP + R"(</span></p>
            <p><strong>Method:</strong> <span class="highlight">)" + requestMethod + R"(</span></p>
            <p><strong>Path:</strong> <span class="highlight">)" + requestPath + R"(</span></p>
            <p><strong>Time:</strong> <span class="highlight">)" + currentTime + R"(</span></p>
        </div>
        
        <h3>🛠️ Technical Features</h3>
        <ul class="tech-list">
            <li>TCP Socket Programming</li>
            <li>HTTP/1.1 Protocol Support</li>
            <li>Multi-client Architecture</li>
            <li>Dynamic Port Allocation</li>
            <li>UTF-8 Character Encoding</li>
            <li>Connection Logging</li>
            <li>Error Handling</li>
        </ul>
        
        <h3>📈 Server Statistics</h3>
        <p>This server has handled <span class="highlight">)" + to_string(connectionCount) + R"(</span> connections since startup.</p>
        <p>Current port: <span class="highlight">)" + to_string(selectedPort) + R"(</span></p>
        
        <div class="footer">
            <p>C++ HTTP Server © 2024 | Educational Project</p>
            <p>Server Time: )" + currentTime + R"(</p>
        </div>
    </div>
    
    <script>
        // Simple JavaScript for time update
        function updateTime() {
            const timeElement = document.querySelector('.footer p:last-child');
            if(timeElement) {
                const now = new Date();
                timeElement.textContent = 'Server Time: ' + now.toLocaleString();
            }
        }
        
        // Update time every minute
        setInterval(updateTime, 60000);
    </script>
</body>
</html>)";

        // Формирование HTTP ответа
        string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: " + to_string(html.length()) + "\r\n"
            "Connection: close\r\n"
            "Server: C++-HTTP-Server/1.0\r\n"
            "\r\n" + html;

        // Отправка ответа клиенту
        int bytesSent = send(clientSocket, response.c_str(), (int)response.length(), 0);

        if (bytesSent > 0) {
            cout << "   📤 Ответ отправлен (" << bytesSent << " байт)" << endl;
        }
        else {
            cerr << "   ❌ Ошибка отправки ответа: " << WSAGetLastError() << endl;
        }

        // Закрытие соединения
        closesocket(clientSocket);
        cout << "   🔒 Соединение закрыто" << endl;
        cout << "   ----------------------------------------\n" << endl;
    }

    // Закрытие серверного сокета (этот код никогда не выполнится)
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}