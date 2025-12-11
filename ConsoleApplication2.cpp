
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <limits>
#include< chrono >
using namespace std;

// ==================== КЛАССЫ И СТРУКТУРЫ ====================

// Структура для хранения информации о книге
struct Book {
    int id;
    string title;
    string author;
    string genre;
    int year;
    bool available;
    string fileName; // Имя файла с содержимым книги (TXT или PDF)

    Book(int _id, string _title, string _author, string _genre, int _year, string _file = "")
        : id(_id), title(_title), author(_author), genre(_genre), year(_year), available(true), fileName(_file) {
    }
};

// Структура для хранения информации о пользователе
struct User {
    int id;
    string fullName;
    string password;
    vector<int> borrowedBooks; // ID взятых книг
    vector<string> activityLog; // Лог действий

    User(int _id, string _name, string _pass)
        : id(_id), fullName(_name), password(_pass) {
    }
};

// Основной класс библиотеки
class LibrarySystem {
private:
    vector<Book> books;
    vector<User> users;
    map<int, User*> userMap; // Для быстрого поиска по ID
    int currentUserId;
    string booksDBFile = "books_database.txt";
    string usersDBFile = "users_database.txt";
    string activityLogFile = "activity_log.txt";

public:
    LibrarySystem() : currentUserId(-1) {
        loadBooksFromFile();
        loadUsersFromFile();
    }

    ~LibrarySystem() {
        saveBooksToFile();
        saveUsersToFile();
    }

    // ==================== МЕТОДЫ ДЛЯ РАБОТЫ С ФАЙЛАМИ ====================

    void loadBooksFromFile() {
        ifstream file(booksDBFile);
        if (!file.is_open()) return;

        int id, year;
        string title, author, genre, fileName;
        bool available;

        while (file >> id) {
            file.ignore();
            getline(file, title);
            getline(file, author);
            getline(file, genre);
            file >> year >> available;
            file.ignore();
            getline(file, fileName);

            Book book(id, title, author, genre, year, fileName);
            book.available = available;
            books.push_back(book);
        }

        file.close();
        cout << "Загружено " << books.size() << " книг из базы данных.\n";
    }

    void saveBooksToFile() {
        ofstream file(booksDBFile);

        for (const auto& book : books) {
            file << book.id << "\n"
                << book.title << "\n"
                << book.author << "\n"
                << book.genre << "\n"
                << book.year << "\n"
                << book.available << "\n"
                << book.fileName << "\n";
        }

        file.close();
    }

    void loadUsersFromFile() {
        ifstream file(usersDBFile);
        if (!file.is_open()) return;

        int id, bookCount;
        string fullName, password;

        while (file >> id) {
            file.ignore();
            getline(file, fullName);
            getline(file, password);

            User user(id, fullName, password);

            file >> bookCount;
            for (int i = 0; i < bookCount; i++) {
                int bookId;
                file >> bookId;
                user.borrowedBooks.push_back(bookId);
            }

            users.push_back(user);
            userMap[id] = &users.back();
        }

        file.close();
        cout << "Загружено " << users.size() << " пользователей из базы данных.\n";
    }

    void saveUsersToFile() {
        ofstream file(usersDBFile);

        for (const auto& user : users) {
            file << user.id << "\n"
                << user.fullName << "\n"
                << user.password << "\n"
                << user.borrowedBooks.size() << " ";

            for (int bookId : user.borrowedBooks) {
                file << bookId << " ";
            }

           
                file << "\n";
        }

        file.close();
    }

    void logActivity(const string& activity) {
        ofstream file(activityLogFile, ios::app);
        if (!file.is_open()) return;

        auto now = chrono::system_clock::now();
        auto now_time_t = chrono::system_clock::to_time_t(now);
        tm localTime;
        localtime_s(&localTime, &now_time_t); // Для Windows

        char timeStr[100];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &localTime);

        file << "[" << timeStr << "] "
            << "Пользователь ID: " << (currentUserId != -1 ? to_string(currentUserId) : "Неизвестен")
            << " - " << activity << "\n";

        file.close();

        // Также сохраняем в лог пользователя
        if (currentUserId != -1 && userMap.count(currentUserId)) {
            userMap[currentUserId]->activityLog.push_back(activity);
        }
    }

    // ==================== МЕТОДЫ РЕГИСТРАЦИИ И ВХОДА ====================

    void registerUser() {
        cout << "\n========== РЕГИСТРАЦИЯ ==========\n";

        string fullName, password;
        cout << "Введите ФИО: ";
        cin.ignore();
        getline(cin, fullName);

        cout << "Введите пароль: ";
        getline(cin, password);

        int newId = users.empty() ? 1 : users.back().id + 1;
        User newUser(newId, fullName, password);
        users.push_back(newUser);
        userMap[newId] = &users.back();

        cout << "\nРегистрация успешна! Ваш ID: " << newId << "\n";
        logActivity("Зарегистрирован новый пользователь: " + fullName);

        saveUsersToFile();
    }

    bool loginUser() {
        cout << "\n========== ВХОД В СИСТЕМУ ==========\n";

        int id;
        string password;

        cout << "Введите ID пользователя: ";
        cin >> id;

        cout << "Введите пароль: ";
        cin.ignore();
        getline(cin, password);

        if (userMap.count(id) && userMap[id]->password == password) {
            currentUserId = id;
            cout << "\nВход выполнен успешно! Добро пожаловать, "
                << userMap[id]->fullName << "!\n";
            logActivity("Пользователь вошел в систему");
            return true;
        }
        else {
            cout << "\nОшибка: Неверный ID или пароль!\n";
            return false;
        }
    }

    void logout() {
        if (currentUserId != -1) {
            logActivity("Пользователь вышел из системы");
            cout << "\nВыход из системы выполнен.\n";
        }
        currentUserId = -1;
    }

    // ==================== МЕТОДЫ ДЛЯ РАБОТЫ С КНИГАМИ ====================

    void addBook() {
        if (currentUserId == -1) {
            cout << "Ошибка: Сначала войдите в систему!\n";
            return;
        }

        cout << "\n========== ДОБАВЛЕНИЕ НОВОЙ КНИГИ ==========\n";

        string title, author, genre, fileName;
        int year;

        cin.ignore();
        cout << "Название книги: ";
        getline(cin, title);

        cout << "Автор: ";
        getline(cin, author);

        cout << "Жанр: ";
        getline(cin, genre);

        cout << "Год издания: ";
        cin >> year;

        cout << "Имя файла с содержимым (TXT/PDF, можно оставить пустым): ";
        cin.ignore();
        getline(cin, fileName);

        int newId = books.empty() ? 1 : books.back().id + 1;
        Book newBook(newId, title, author, genre, year, fileName);
        books.push_back(newBook);

        cout << "\nКнига успешно добавлена! ID книги: " << newId << "\n";
        logActivity("Добавлена новая книга: " + title);

        saveBooksToFile();
    }

    void displayAllBooks() {
        cout << "\n========== КАТАЛОГ КНИГ ==========\n";
        cout << "Всего книг: " << books.size() << "\n\n";

        for (const auto& book : books) {
            cout << "ID: " << book.id << "\n"
                << "Название: " << book.title << "\n"
                << "Автор: " << book.author << "\n"
                << "Жанр: " << book.genre << "\n"
                << "Год: " << book.year << "\n"

               
                << "Доступность: " << (book.available ? "✓ В наличии" : "✗ Выдана") << "\n";

            if (!book.fileName.empty()) {
                cout << "Файл: " << book.fileName << "\n";
            }
            cout << "----------------------------------------\n";
        }
    }

    void searchBooks() {
        cout << "\n========== ПОИСК КНИГ ==========\n";
        cout << "1. По названию\n";
        cout << "2. По автору\n";
        cout << "3. По жанру\n";
        cout << "Выберите вариант: ";

        int choice;
        cin >> choice;
        cin.ignore();

        string searchTerm;
        cout << "Введите поисковый запрос: ";
        getline(cin, searchTerm);

        transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(), ::tolower);

        vector<Book*> foundBooks;

        for (auto& book : books) {
            string titleLower = book.title;
            string authorLower = book.author;
            string genreLower = book.genre;

            transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            transform(authorLower.begin(), authorLower.end(), authorLower.begin(), ::tolower);
            transform(genreLower.begin(), genreLower.end(), genreLower.begin(), ::tolower);

            bool match = false;

            switch (choice) {
            case 1: match = titleLower.find(searchTerm) != string::npos; break;
            case 2: match = authorLower.find(searchTerm) != string::npos; break;
            case 3: match = genreLower.find(searchTerm) != string::npos; break;
            default: return;
            }

            if (match) {
                foundBooks.push_back(&book);
            }
        }

        if (foundBooks.empty()) {
            cout << "\nКниги не найдены.\n";
        }
        else {
            cout << "\nНайдено " << foundBooks.size() << " книг:\n";
            for (const auto& book : foundBooks) {
                cout << "ID: " << book->id << " - \"" << book->title << "\" ("
                    << book->author << ") - "
                    << (book->available ? "В наличии" : "Выдана") << "\n";
            }
        }
    }

    void borrowBook() {
        if (currentUserId == -1) {
            cout << "Ошибка: Сначала войдите в систему!\n";
            return;
        }

        cout << "\n========== ВЗЯТИЕ КНИГИ ==========\n";
        displayAllBooks();

        int bookId;
        cout << "\nВведите ID книги для взятия: ";
        cin >> bookId;

        auto it = find_if(books.begin(), books.end(),
            [bookId](const Book& b) { return b.id == bookId; });

        if (it == books.end()) {
            cout << "Ошибка: Книга с таким ID не найдена!\n";
            return;
        }

        if (!it->available) {
            cout << "Ошибка: Эта книга уже выдана!\n";
            return;
        }

        // Проверяем, не превышает ли пользователь лимит
        User* user = userMap[currentUserId];
        if (user->borrowedBooks.size() >= 5) {
            cout << "Ошибка: Вы не можете взять более 5 книг одновременно!\n";
            return;
        }

        it->available = false;
        user->borrowedBooks.push_back(bookId);

        cout << "\nКнига \"" << it->title << "\" успешно взята!\n";
        logActivity("Взял книгу: " + it->title + " (ID: " + to_string(bookId) + ")");

        saveBooksToFile();
        saveUsersToFile();
    }

    void returnBook() {
        if (currentUserId == -1) {
            cout << "Ошибка: Сначала войдите в систему!\n";
            return;
        }

        User* user = userMap[currentUserId];

        if (user->borrowedBooks.empty()) {
            cout << "У вас нет взятых книг.\n";
            return;
        }

        cout << "\n========== ВОЗВРАТ КНИГИ ==========\n";
        cout << "Ваши взятые книги:\n";

  
            for (int bookId : user->borrowedBooks) {
                auto it = find_if(books.begin(), books.end(),
                    [bookId](const Book& b) { return b.id == bookId; });
                if (it != books.end()) {
                    cout << "ID: " << bookId << " - \"" << it->title << "\"\n";
                }
            }

        int bookId;
        cout << "\nВведите ID книги для возврата: ";
        cin >> bookId;

        auto bookIt = find(user->borrowedBooks.begin(), user->borrowedBooks.end(), bookId);
        if (bookIt == user->borrowedBooks.end()) {
            cout << "Ошибка: У вас нет книги с таким ID!\n";
            return;
        }

        // Находим книгу и отмечаем как доступную
        auto libIt = find_if(books.begin(), books.end(),
            [bookId](const Book& b) { return b.id == bookId; });

        if (libIt != books.end()) {
            libIt->available = true;
            user->borrowedBooks.erase(bookIt);

            cout << "\nКнига \"" << libIt->title << "\" успешно возвращена!\n";
            logActivity("Вернул книгу: " + libIt->title + " (ID: " + to_string(bookId) + ")");

            saveBooksToFile();
            saveUsersToFile();
        }
    }

    void displayUserInfo() {
        if (currentUserId == -1) {
            cout << "Ошибка: Сначала войдите в систему!\n";
            return;
        }

        User* user = userMap[currentUserId];

        cout << "\n========== ВАШ ПРОФИЛЬ ==========\n";
        cout << "ID: " << user->id << "\n";
        cout << "ФИО: " << user->fullName << "\n";
        cout << "Взято книг: " << user->borrowedBooks.size() << "\n";

        if (!user->borrowedBooks.empty()) {
            cout << "\nСписок взятых книг:\n";
            for (int bookId : user->borrowedBooks) {
                auto it = find_if(books.begin(), books.end(),
                    [bookId](const Book& b) { return b.id == bookId; });
                if (it != books.end()) {
                    cout << "  • " << it->title << " (ID: " << bookId << ")\n";
                }
            }
        }

        if (!user->activityLog.empty()) {
            cout << "\nПоследние действия:\n";
            int start = max(0, (int)user->activityLog.size() - 5);
            for (int i = start; i < user->activityLog.size(); i++) {
                cout << "  • " << user->activityLog[i] << "\n";
            }
        }
    }

    void viewBookContent() {
        cout << "\n========== ЧТЕНИЕ КНИГИ ==========\n";

        int bookId;
        cout << "Введите ID книги для чтения: ";
        cin >> bookId;

        auto it = find_if(books.begin(), books.end(),
            [bookId](const Book& b) { return b.id == bookId; });

        if (it == books.end()) {
            cout << "Ошибка: Книга с таким ID не найдена!\n";
            return;
        }

        if (it->fileName.empty()) {
            cout << "Для этой книги нет электронной версии.\n";
            return;
        }

        cout << "\nОткрываю файл: " << it->fileName << "\n";
        cout << "Содержимое книги \"" << it->title << "\":\n";
        cout << "========================================\n";

        ifstream file(it->fileName);
        if (file.is_open()) {
            string line;
            int lineCount = 0;
            while (getline(file, line) && lineCount < 50) { // Показываем первые 50 строк
                cout << line << "\n";
                lineCount++;
            }
            if (lineCount >= 50) {
                cout << "\n... (показаны первые 50 строк)\n";
            }
            file.close();
        }
        else {
            cout << "Не удалось открыть файл.\n";
        }

        if (currentUserId != -1) {
            logActivity("Читал книгу: " + it->title + " (ID: " + to_string(bookId) + ")");
        }
    }

    // ==================== ГЛАВНОЕ МЕНЮ ====================

    void mainMenu() {
        while (true) {

          
                cout << "\n========== БИБЛИОТЕЧНАЯ СИСТЕМА ==========\n";

            if (currentUserId == -1) {
                cout << "1. Регистрация\n";
                cout << "2. Вход\n";
                cout << "3. Просмотр каталога книг\n";
                cout << "4. Поиск книг\n";
                cout << "5. Выход\n";
            }
            else {
                cout << "Добро пожаловать, " << userMap[currentUserId]->fullName << "!\n";
                cout << "1. Просмотр каталога книг\n";
                cout << "2. Поиск книг\n";
                cout << "3. Взять книгу\n";
                cout << "4. Вернуть книгу\n";
                cout << "5. Добавить книгу (админ)\n";
                cout << "6. Читать книгу\n";
                cout << "7. Мой профиль\n";
                cout << "8. Выход из системы\n";
                cout << "9. Выход из программы\n";
            }

            cout << "\nВыберите действие: ";

            int choice;
            cin >> choice;

            if (currentUserId == -1) {
                switch (choice) {
                case 1: registerUser(); break;
                case 2: loginUser(); break;
                case 3: displayAllBooks(); break;
                case 4: searchBooks(); break;
                case 5:
                    cout << "\nДо свидания!\n";
                    return;
                default:
                    cout << "Неверный выбор!\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
            else {
                switch (choice) {
                case 1: displayAllBooks(); break;
                case 2: searchBooks(); break;
                case 3: borrowBook(); break;
                case 4: returnBook(); break;
                case 5: addBook(); break;
                case 6: viewBookContent(); break;
                case 7: displayUserInfo(); break;
                case 8: logout(); break;
                case 9:
                    logout();
                    cout << "\nДо свидания!\n";
                    return;
                default:
                    cout << "Неверный выбор!\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
        }
    }
};

// ==================== ОСНОВНАЯ ФУНКЦИЯ ====================

int main() {
    // Устанавливаем русскую локаль для корректного отображения текста
    setlocale(LC_ALL, "Russian");

    cout << "=== СИСТЕМА УПРАВЛЕНИЯ БИБЛИОТЕКОЙ ===\n";
    cout << "Версия 1.0\n";
    cout << "Разработано по вашей схеме\n\n";

    // Добавляем несколько тестовых книг, если база пуста
    LibrarySystem library;

    // Запускаем главное меню
    library.mainMenu();

    return 0;
}