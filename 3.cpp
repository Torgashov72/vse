
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#include <ws2tcpip.h>  // Добавьте этот заголовочный файл!

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    // Устанавливаем русскую кодировку для консоли
    system("chcp 1251 > nul");

    cout << "=== HTTP-КЛИЕНТ НА C++ ===" << endl;
    cout << "Клиент для отправки запросов к веб-серверам\n" << endl;

    // Инициализация WinSock (один раз при запуске)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "❌ Ошибка инициализации WinSock!" << endl;
        return 1;
    }
    cout << "✓ WinSock инициализирован\n" << endl;

    while (true) {
        cout << "           МЕНЮ КЛИЕНТА               " << endl;
        cout << " 1. Отправить запрос к сайту         " << endl;
        cout << " 2. Примеры сайтов для теста         " << endl;
        cout << " 3. Тест локального сервера (localhost:8080)" << endl;
        cout << " 4. Выйти из программы               " << endl;
        cout << "Ваш выбор: ";

        string choice;
        getline(cin, choice);

        if (choice == "4") {
            cout << "Выход из программы. До свидания!" << endl;
            break;
        }

        if (choice == "2") {
            cout << "\n📋 Примеры сайтов для тестирования:" << endl;
            cout << "----------------------------------------" << endl;
            cout << "1. example.com      - Тестовый сайт" << endl;
            cout << "2. httpbin.org      - Отладочный HTTP сервис" << endl;
            cout << "3. neverssl.com     - Работает без SSL" << endl;
            cout << "4. google.com       - Поисковая система" << endl;
            cout << "5. yandex.ru        - Российский поисковик" << endl;
            cout << "\n💡 Введите адрес без http:// (например: example.com)" << endl;
            continue;
        }

        if (choice == "3") {
            // Тестируем локальный сервер
            cout << "\n🔍 Тестирование локального сервера на порту 8080..." << endl;

            SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (clientSocket == INVALID_SOCKET) {
                cerr << "❌ Ошибка создания сокета!" << endl;
                continue;
            }

            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(8080);  // Порт вашего сервера

            // Для localhost используем 127.0.0.1
            serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

            cout << "⏳ Подключаюсь к 127.0.0.1:8080..." << endl;

            if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                int error = WSAGetLastError();
                cerr << "❌ Не удалось подключиться к localhost:8080!" << endl;
                cerr << "Код ошибки: " << error << endl;
                cerr << "Убедитесь, что сервер запущен и слушает порт 8080" << endl;
                closesocket(clientSocket);
                continue;
            }

            cout << "✅ Подключение установлено!" << endl;

            // Формируем запрос
            string httpRequest =
                "GET / HTTP/1.1\r\n"
                "Host: localhost:8080\r\n"
                "User-Agent: Test-Client/1.0\r\n"
                "Connection: close\r\n"
                "\r\n";

            // Отправляем запрос
            send(clientSocket, httpRequest.c_str(), httpRequest.length(), 0);
            cout << "📤 Запрос отправлен" << endl;

            // Получаем ответ
            char buffer[4096];
            int totalBytes = 0;
            bool gotResponse = false;

            // Устанавливаем неблокирующий режим с таймаутом
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(clientSocket, &readSet);

            struct timeval timeout;
            timeout.tv_sec = 5;  // 5 секунд
            timeout.tv_usec = 0;

            int selectResult = select(0, &readSet, NULL, NULL, &timeout);

            if (selectResult > 0) {

                
                    while (true) {
                        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

                        if (bytesReceived > 0) {
                            buffer[bytesReceived] = '\0';
                            cout << "📥 Получено " << bytesReceived << " байт:" << endl;
                            cout << "========================================" << endl;
                            cout << buffer << endl;
                            cout << "========================================" << endl;
                            totalBytes += bytesReceived;
                            gotResponse = true;
                        }
                        else if (bytesReceived == 0) {
                            // Соединение закрыто
                            break;
                        }
                        else {
                            // Ошибка
                            break;
                        }
                    }
            }
            else if (selectResult == 0) {
                cout << "⚠️ Таймаут ожидания ответа (5 секунд)" << endl;
            }

            if (gotResponse) {
                cout << "✅ Ответ получен! Всего байт: " << totalBytes << endl;
            }
            else {
                cout << "❌ Ответ не получен" << endl;
            }

            closesocket(clientSocket);
            cout << "\n🔒 Соединение закрыто" << endl;
            cout << "────────────────────────────────────────\n" << endl;
            continue;
        }

        if (choice != "1") {
            cout << "⚠️ Неверный выбор! Попробуйте снова." << endl;
            continue;
        }

        // Получаем адрес сайта от пользователя
        string host;
        cout << "\nВведите адрес сайта (или IP): ";
        getline(cin, host);

        if (host.empty()) {
            cout << "❌ Адрес не может быть пустым!" << endl;
            continue;
        }

        // Удаляем протокол если есть
        size_t pos;
        if ((pos = host.find("http://")) != string::npos) {
            host = host.substr(pos + 7);
        }
        if ((pos = host.find("https://")) != string::npos) {
            cout << "⚠️ Внимание: HTTPS не поддерживается, используйте HTTP" << endl;
            host = host.substr(pos + 8);
        }

        // Удаляем путь если есть
        if ((pos = host.find('/')) != string::npos) {
            host = host.substr(0, pos);
        }

        cout << "\n🔗 Подключаюсь к: " << host << endl;

        // Создание сокета
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "❌ Ошибка создания сокета!" << endl;
            continue;
        }

        // Настройка адреса сервера
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(80);  // HTTP порт

        // Проверяем, это IP или доменное имя
        unsigned long ip = inet_addr(host.c_str());

        if (ip != INADDR_NONE) {
            // Это IP адрес
            serverAddr.sin_addr.s_addr = ip;
            cout << "✓ Используем IP адрес: " << host << endl;
        }
        else {
            // Это доменное имя - нужно разрешить в IP
            cout << "🔍 Разрешаю доменное имя в IP..." << endl;

            hostent* remoteHost = gethostbyname(host.c_str());
            if (remoteHost == nullptr) {
                cerr << "❌ Не удалось разрешить доменное имя: " << host << endl;
                cerr << "Код ошибки: " << WSAGetLastError() << endl;
                closesocket(clientSocket);
                continue;
            }

            serverAddr.sin_addr.s_addr = *((unsigned long*)remoteHost->h_addr_list[0]);

            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, remoteHost->h_addr_list[0], ipStr, sizeof(ipStr));
            cout << "✓ Доменное имя разрешено в IP: " << ipStr << endl;
        }

        // Устанавливаем таймаут подключения
        int timeout = 5000; // 5 секунд
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        // Подключение к серверу
        cout << "⏳ Устанавливаю соединение..." << endl;

        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            int error = WSAGetLastError();
            cerr << "❌ Не удалось подключиться к серверу!" << endl;
            cerr << "Код ошибки: " << error << endl;
            cerr << "Возможные причины:" << endl;
            cerr << "1. Сайт недоступен" << endl;
            cerr << "2. Проблемы с интернет-соединением" << endl;
            cerr << "3. Фаервол блокирует подключение" << endl;
            cerr << "4. Сайт использует HTTPS (порт 443)" << endl;
            closesocket(clientSocket);
            continue;
        }

        cout << "✅ Успешно подключился к серверу!" << endl;

        // Формирование HTTP GET запроса
        string httpRequest =
            "GET / HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "User-Agent: C++-HTTP-Client/1.0\r\n"
            "Accept: text/html\r\n"
            "Accept-Language: ru-RU,ru;q=0.9\r\n"
            "Connection: close\r\n"
            "\r\n";

        // Отправка запроса
        cout << "\n📤 Отправляю HTTP-запрос..." << endl;

        int bytesSent = send(clientSocket, httpRequest.c_str(), (int)httpRequest.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            cerr << "❌ Ошибка отправки запроса!" << endl;
            closesocket(clientSocket);
            continue;
        }

        cout << "✓ Отправлено байт: " << bytesSent << endl;

        // Получение ответа
        cout << "\n⏳ Ожидаю ответ от сервера..." << endl;
        cout << "========================================" << endl;

        char buffer[8192];  // Увеличим буфер
        int totalBytes = 0;
        bool gotResponse = false;
        int timeoutCounter = 0;

        // Читаем ответ частями
        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';

                // Показываем только первые 2000 символов
                if (totalBytes == 0) {
                    // Показываем заголовки
                    string response(buffer);
                    size_t headerEnd = response.find("\r\n\r\n");
                    if (headerEnd != string::npos) {
                        cout << "📋 ЗАГОЛОВКИ ОТВЕТА:" << endl;
                        cout << response.substr(0, headerEnd) << endl;
                        cout << "\n📄 ТЕЛО ОТВЕТА (первые 2000 символов):" << endl;
                        cout << response.substr(headerEnd + 4, 2000) << endl;
                        if (response.length() > headerEnd + 4 + 2000) {
                            cout << "\n... [далее пропущено] ..." << endl;
                        }
                    }
                    else {
                        cout << buffer;
                    }
                }

                totalBytes += bytesReceived;
                gotResponse = true;
                timeoutCounter = 0;
            }
            else if (bytesReceived == 0) {
                // Соединение закрыто сервером
                break;
            }
            else {
                // Ошибка или таймаут
                timeoutCounter++;
                if (timeoutCounter > 3) break; // 3 неудачные попытки
                Sleep(100); // Ждем 100ms перед повторной попыткой
            }
        }

        cout << "========================================" << endl;

        if (gotResponse) {
            cout << "\n✅ Ответ получен успешно!" << endl;
            cout << "📊 Всего получено байт: " << totalBytes << endl;
        }
        else {
            cout << "\n❌ Сервер не ответил или ответ пустой" << endl;
            cout << "Возможные причины:" << endl;
            cout << "1. Сервер не поддерживает HTTP/1.1" << endl;
            cout << "2. Требуется HTTPS соединение" << endl;
            cout << "3. Сервер закрыл соединение" << endl;
            cout <<

                
                "4. Проблемы с сетью" << endl;
        }

        // Закрытие соединения
        closesocket(clientSocket);

        cout << "\n🔒 Соединение закрыто" << endl;
        cout << "────────────────────────────────────────\n" << endl;
    }

    // Завершение работы WinSock
    WSACleanup();

    return 0;
}