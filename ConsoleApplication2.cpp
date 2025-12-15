#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <conio.h>
#include <windows.h>
using namespace std;

// ==================== СТРУКТУРЫ ====================

struct Book {
    int id;
    string title;
    string author;
    string genre;
    int year;
    string annotation;
    bool available;

    Book(int _id = 0, string _title = "", string _author = "", string _genre = "",
        int _year = 0, string _annotation = "")
        : id(_id), title(_title), author(_author), genre(_genre),
        year(_year), annotation(_annotation), available(true) {
    }
};

struct User {
    int id;
    string fullName;
    string password;
    bool isAdmin;
    vector<int> borrowedBooks;

    User(int _id = 0, string _name = "", string _pass = "", bool _admin = false)
        : id(_id), fullName(_name), password(_pass), isAdmin(_admin) {
    }
};

// ==================== КЛАСС ДЛЯ НАВИГАЦИИ МЕНЮ ====================

class Menu {
private:
    vector<string> options;
    int currentSelection;
    string title;

    void display() {
        system("cls");
        cout << "========== " << title << " ==========\n";

        for (int i = 0; i < options.size(); i++) {
            if (i == currentSelection) {
                cout << "> " << options[i] << " <\n";
            }
            else {
                cout << "  " << options[i] << "\n";
            }
        }
        cout << "\nИспользуйте стрелки ВВЕРХ/ВНИЗ для навигации, ENTER для выбора\n";
    }

public:
    Menu(string _title, vector<string> _options)
        : title(_title), options(_options), currentSelection(0) {
    }

    int run() {
        int key;
        do {
            display();
            key = _getch();

            if (key == 224) {
                key = _getch();
                switch (key) {
                case 72: // Стрелка вверх
                    currentSelection = (currentSelection - 1 + options.size()) % options.size();
                    break;
                case 80: // Стрелка вниз
                    currentSelection = (currentSelection + 1) % options.size();
                    break;
                }
            }
        } while (key != 13); // Enter
        return currentSelection;
    }
};

// ==================== ОСНОВНОЙ КЛАСС БИБЛИОТЕКИ ====================

class LibrarySystem {
private:
    vector<Book> books;
    vector<User> users;
    map<int, int> userIdToIndex;
    int currentUserId;
    string usersFile = "users_data.txt";
    string booksFile = "books_data.txt";

    // Данные администратора (хранятся в коде)
    const int ADMIN_ID = 1;
    const string ADMIN_NAME = "Администратор";
    const string ADMIN_PASSWORD = "admin123";

    void createDefaultAdmin() {
        // Проверяем, нет ли уже администратора
        if (userIdToIndex.find(ADMIN_ID) == userIdToIndex.end()) {
            User admin(ADMIN_ID, ADMIN_NAME, ADMIN_PASSWORD, true);
            users.push_back(admin);
            userIdToIndex[ADMIN_ID] = users.size() - 1;
        }
    }

    void saveUsersData() {
        ofstream file(usersFile);
        if (!file.is_open()) {
            cout << "Ошибка: не удалось открыть файл пользователей для записи\n";
            return;
        }

        // Сохраняем только обычных пользователей (не администратора)
        int regularUsersCount = 0;
        for (const auto& user : users) {
            if (!user.isAdmin) {
                regularUsersCount++;
            }
        }

        file << regularUsersCount << "\n";
        for (const auto& user : users) {
            // Не сохраняем администратора в файл
            if (user.isAdmin) {
                continue;
            }

            file << user.id << "\n"
                << user.fullName << "\n"
                << user.password << "\n"
                << user.isAdmin << "\n"
                << user.borrowedBooks.size() << " ";
            for (int bookId : user.borrowedBooks) {
                file << bookId << " ";
            }
            file << "\n";
        }
        file.close();
    }

    void loadUsersData() {
        ifstream file(usersFile);
        
        // Всегда создаем администратора из кода
        createDefaultAdmin();
        
        // Если файла нет, просто создаем администратора и выходим
        if (!file.is_open()) {
            return;
        }

        int userCount;
        file >> userCount;
        file.ignore();

        for (int i = 0; i < userCount; i++) {
            int id, bookCount;
            string fullName, password;
            bool isAdmin;

            file >> id;
            file.ignore();
            getline(file, fullName);
            getline(file, password);
            file >> isAdmin;
            file >> bookCount;

            // Проверяем, не конфликтует ли ID с администратором
            if (id == ADMIN_ID) {
                // Пропускаем этого пользователя из файла (админ уже есть в коде)
                for (int j = 0; j < bookCount; j++) {
                    int temp;
                    file >> temp;
                }
                file.ignore();
                continue;
            }

            User user(id, fullName, password, isAdmin);
            
            for (int j = 0; j < bookCount; j++) {
                int bookId;
                file >> bookId;
                user.borrowedBooks.push_back(bookId);
            }
            file.ignore();

            users.push_back(user);
            userIdToIndex[id] = users.size() - 1;
        }
        file.close();
    }

    void saveBooksData() {
        ofstream file(booksFile);
        if (!file.is_open()) {
            cout << "Ошибка: не удалось открыть файл книг для записи\n";
            return;
        }

        file << books.size() << "\n";
        for (const auto& book : books) {
            string annotationForFile = book.annotation;
            replace(annotationForFile.begin(), annotationForFile.end(), '\n', '|');

            file << book.id << "\n"
                << book.title << "\n"
                << book.author << "\n"
                << book.genre << "\n"
                << book.year << "\n"
                << annotationForFile << "\n"
                << book.available << "\n";
        }
        file.close();
    }

    void loadBooksData() {
        ifstream file(booksFile);
        if (!file.is_open()) {
            // Создаем книги с аннотациями
            books.push_back(Book(1, "Война и мир", "Лев Толстой", "Роман", 1869,
                "Монументальный роман-эпопея Льва Толстого, описывающий русское общество в эпоху войн против Наполеона.\nКлючевые темы: любовь, честь, патриотизм, поиск смысла жизни."));

            books.push_back(Book(2, "Преступление и наказание", "Фёдор Достоевский", "Роман", 1866,
                "Психологический роман о бывшем студенте Родионе Раскольникове, который совершает убийство,\nчтобы проверить свою теорию о 'необыкновенных' людях, имеющих право преступать моральные законы."));

            books.push_back(Book(3, "Мастер и Маргарита", "Михаил Булгаков", "Фэнтези", 1967,
                "Мистический роман, сочетающий сатиру на советскую действительность 1930-х годов\nс философскими размышлениями о добре и зле, истине и творчестве."));

            books.push_back(Book(4, "1984", "Джордж Оруэлл", "Антиутопия", 1949,
                "Классическая антиутопия о тоталитарном обществе под контролем 'Старшего Брата'.\nРоман ввел в обиход такие понятия как 'Большой Брат', 'новояз' и 'двоемыслие'."));

            books.push_back(Book(5, "Гарри Поттер и философский камень", "Джоан Роулинг", "Фэнтези", 1997,
                "Первая книга знаменитой серии о юном волшебнике Гарри Поттере.\nМальчик-сирота discovers he is a wizard and begins his education at Hogwarts School of Witchcraft and Wizardry."));

            saveBooksData();
            return;
        }

        books.clear();

        int bookCount;
        file >> bookCount;
        file.ignore();

        for (int i = 0; i < bookCount; i++) {
            int id, year;
            string title, author, genre, annotation;
            bool available;

            file >> id;
            file.ignore();
            getline(file, title);
            getline(file, author);
            getline(file, genre);
            file >> year;
            file.ignore();
            getline(file, annotation);
            file >> available;
            file.ignore();

            replace(annotation.begin(), annotation.end(), '|', '\n');

            Book book(id, title, author, genre, year, annotation);
            book.available = available;
            books.push_back(book);
        }
        file.close();
    }

    void saveAllData() {
        saveUsersData();  // Сохраняет только обычных пользователей
        saveBooksData();
    }

    void loadAllData() {
        // Загружаем книги
        loadBooksData();
        
        // Загружаем пользователей (админ уже создан в loadUsersData)
        loadUsersData();
        
        // Проверяем, есть ли администратор
        createDefaultAdmin();
    }

    void waitForAnyKey() {
        cout << "\nНажмите любую клавишу для продолжения...";
        _getch();
    }

    // Получение пользователя по ID
    User* getUserById(int id) {
        if (userIdToIndex.find(id) == userIdToIndex.end()) {
            return nullptr;
        }
        int index = userIdToIndex[id];
        if (index >= 0 && index < users.size()) {
            return &users[index];
        }
        return nullptr;
    }

    // Получение текущего пользователя
    User* getCurrentUser() {
        return getUserById(currentUserId);
    }

public:
    LibrarySystem() : currentUserId(-1) {
        loadAllData();
    }

    ~LibrarySystem() {
        saveAllData();
    }

    // ==================== МЕТОДЫ РЕГИСТРАЦИИ И ВХОДА ====================

    void registerUser() {
        system("cls");
        cout << "========== РЕГИСТРАЦИЯ ==========\n";

        string fullName, password;
        cout << "Введите ФИО: ";
        if (cin.peek() == '\n') cin.ignore();
        getline(cin, fullName);

        if (fullName.empty()) {
            cout << "\nОшибка: ФИО не может быть пустым!\n";
            Sleep(1500);
            return;
        }

        cout << "Введите пароль: ";
        getline(cin, password);

        if (password.empty()) {
            cout << "\nОшибка: Пароль не может быть пустым!\n";
            Sleep(1500);
            return;
        }

        // Находим максимальный ID среди пользователей (кроме администратора)
        int maxId = ADMIN_ID;  // Начинаем с ID администратора
        for (const auto& user : users) {
            if (user.id > maxId) {
                maxId = user.id;
            }
        }
        
        int newId = maxId + 1;
        users.push_back(User(newId, fullName, password, false));
        userIdToIndex[newId] = users.size() - 1;

        cout << "\nРегистрация успешна! Ваш ID: " << newId << "\n";
        Sleep(1500);

        saveUsersData();  // Сохраняем только обычных пользователей
    }

    bool loginUser() {
        system("cls");
        cout << "========== ВХОД В СИСТЕМУ ==========\n";

        int id;
        string password;

        cout << "Введите ID пользователя: ";
        if (!(cin >> id)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\nОшибка: Неверный формат ID!\n";
            Sleep(1500);
            return false;
        }

        cout << "Введите пароль: ";
        cin.ignore();
        getline(cin, password);

        // Специальная проверка для администратора (из кода)
        if (id == ADMIN_ID && password == ADMIN_PASSWORD) {
            // Проверяем, существует ли уже администратор в системе
            User* existingAdmin = getUserById(ADMIN_ID);
            if (existingAdmin == nullptr) {
                // Создаем администратора, если его нет
                users.push_back(User(ADMIN_ID, ADMIN_NAME, ADMIN_PASSWORD, true));
                userIdToIndex[ADMIN_ID] = users.size() - 1;
            }
            
            currentUserId = ADMIN_ID;
            cout << "\nВход выполнен успешно! Добро пожаловать, " << ADMIN_NAME << "!\n";
            Sleep(1500);
            return true;
        }

        // Проверка для обычных пользователей
        User* user = getUserById(id);
        if (user != nullptr && user->password == password) {
            currentUserId = id;
            cout << "\nВход выполнен успешно! Добро пожаловать, "
                << user->fullName << "!\n";
            Sleep(1500);
            return true;
        }
        else {
            cout << "\nОшибка: Неверный ID или пароль!\n";
            Sleep(1500);
            return false;
        }
    }

    void logout() {
        currentUserId = -1;
        cout << "\nВыход из системы выполнен.\n";
        Sleep(1000);
    }

    // ==================== МЕТОДЫ ДЛЯ РАБОТЫ С КНИГАМИ ====================

    void addBook() {
        system("cls");
        cout << "========== ДОБАВЛЕНИЕ НОВОЙ КНИГИ ==========\n";

        string title, author, genre, annotation;
        int year;

        cin.ignore();
        cout << "Название книги: ";
        getline(cin, title);

        if (title.empty()) {
            cout << "\nОшибка: Название не может быть пустым!\n";
            waitForAnyKey();
            return;
        }

        cout << "Автор: ";
        getline(cin, author);

        cout << "Жанр: ";
        getline(cin, genre);

        cout << "Год издания: ";
        if (!(cin >> year)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\nОшибка: Неверный формат года!\n";
            waitForAnyKey();
            return;
        }

        cin.ignore();
        cout << "\nВведите аннотацию к книге (для завершения введите точку на отдельной строке):\n";
        cout << "Можно вводить несколько строк:\n";

        string line;
        while (true) {
            getline(cin, line);
            if (line == ".") {
                break;
            }
            if (!annotation.empty()) {
                annotation += "\n";
            }
            annotation += line;
        }

        int newId = books.empty() ? 1 : books.back().id + 1;
        books.push_back(Book(newId, title, author, genre, year, annotation));

        cout << "\nКнига успешно добавлена! ID книги: " << newId << "\n";
        Sleep(1500);

        saveBooksData();
    }

    void removeBook() {
        system("cls");
        cout << "========== УДАЛЕНИЕ КНИГИ ==========\n";

        if (books.empty()) {
            cout << "В библиотеке нет книг для удаления.\n";
            waitForAnyKey();
            return;
        }

        cout << "Список книг:\n\n";
        for (const auto& book : books) {
            cout << "ID: " << book.id << " - \"" << book.title << "\"";
            if (!book.available) cout << " [ВЫДАНА]";
            cout << "\n";
        }

        cout << "\nВведите ID книги для удаления (0 - отмена): ";
        int bookId;
        if (!(cin >> bookId)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\nОшибка: Неверный формат ID!\n";
            waitForAnyKey();
            return;
        }

        if (bookId == 0) return;

        auto it = find_if(books.begin(), books.end(),
            [bookId](const Book& b) { return b.id == bookId; });

        if (it == books.end()) {
            cout << "\nОшибка: Книга с таким ID не найдена!\n";
            Sleep(1500);
            return;
        }

        if (!it->available) {
            cout << "\nОшибка: Книга в данный момент выдана читателю!\n";
            Sleep(1500);
            return;
        }

        string bookTitle = it->title;
        books.erase(it);

        cout << "\nКнига \"" << bookTitle << "\" успешно удалена!\n";
        Sleep(1500);

        saveBooksData();
    }

    void displayAllBooks() {
        system("cls");
        cout << "========== КАТАЛОГ КНИГ ==========\n";
        cout << "Всего книг: " << books.size() << "\n\n";

        if (books.empty()) {
            cout << "В библиотеке пока нет книг.\n";
        }
        else {
            for (const auto& book : books) {
                cout << "ID: " << book.id << "\n"
                    << "Название: " << book.title << "\n"
                    << "Автор: " << book.author << "\n"
                    << "Жанр: " << book.genre << "\n"
                    << "Год: " << book.year << "\n";

                if (!book.annotation.empty()) {
                    cout << "Аннотация: " << book.annotation << "\n";
                }

                cout << "Статус: " << (book.available ? "В наличии" : "Выдана") << "\n";
                cout << "----------------------------------------\n";
            }
        }

        waitForAnyKey();
    }

    void viewBookDetails() {
        system("cls");
        cout << "========== ПОДРОБНАЯ ИНФОРМАЦИЯ О КНИГЕ ==========\n";

        if (books.empty()) {
            cout << "В библиотеке пока нет книг.\n";
            waitForAnyKey();
            return;
        }

        cout << "Введите ID книги для просмотра (0 - отмена): ";
        int bookId;
        if (!(cin >> bookId)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\nОшибка: Неверный формат ID!\n";
            waitForAnyKey();
            return;
        }

        if (bookId == 0) return;

        auto it = find_if(books.begin(), books.end(),
            [bookId](const Book& b) { return b.id == bookId; });

        if (it == books.end()) {
            cout << "\nОшибка: Книга с таким ID не найдена!\n";
            Sleep(1500);
            return;
        }

        system("cls");
        cout << "========== ПОДРОБНАЯ ИНФОРМАЦИЯ О КНИГЕ ==========\n\n";
        cout << "ID: " << it->id << "\n";
        cout << "Название: " << it->title << "\n";
        cout << "Автор: " << it->author << "\n";
        cout << "Жанр: " << it->genre << "\n";
        cout << "Год издания: " << it->year << "\n";
        cout << "Статус: " << (it->available ? "В наличии" : "Выдана") << "\n\n";

        if (!it->annotation.empty()) {
            cout << "АННОТАЦИЯ:\n";
            cout << "========================================\n";
            cout << it->annotation << "\n";
            cout << "========================================\n";
        }
        else {
            cout << "Аннотация: отсутствует\n";
        }

        waitForAnyKey();
    }

    void searchBooks() {
        vector<string> searchOptions = {
            "По названию",
            "По автору",
            "По жанру",
            "По аннотации",
            "Вернуться в меню"
        };

        Menu searchMenu("ПОИСК КНИГ", searchOptions);
        int choice = searchMenu.run();

        if (choice == 4) return;

        system("cls");
        cout << "========== ПОИСК КНИГ ==========\n";

        string searchTerm;
        cout << "Введите поисковый запрос: ";
        cin.ignore();
        getline(cin, searchTerm);

        if (searchTerm.empty()) {
            cout << "\nОшибка: Поисковый запрос не может быть пустым!\n";
            waitForAnyKey();
            return;
        }

        transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(), ::tolower);

        vector<Book*> foundBooks;

        for (auto& book : books) {
            string titleLower = book.title;
            string authorLower = book.author;
            string genreLower = book.genre;
            string annotationLower = book.annotation;

            transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            transform(authorLower.begin(), authorLower.end(), authorLower.begin(), ::tolower);
            transform(genreLower.begin(), genreLower.end(), genreLower.begin(), ::tolower);
            transform(annotationLower.begin(), annotationLower.end(), annotationLower.begin(), ::tolower);

            bool match = false;
            switch (choice) {
            case 0: match = titleLower.find(searchTerm) != string::npos; break;
            case 1: match = authorLower.find(searchTerm) != string::npos; break;
            case 2: match = genreLower.find(searchTerm) != string::npos; break;
            case 3: match = annotationLower.find(searchTerm) != string::npos; break;
            }

            if (match) {
                foundBooks.push_back(&book);
            }
        }

        system("cls");
        cout << "========== РЕЗУЛЬТАТЫ ПОИСКА ==========\n";

        if (foundBooks.empty()) {
            cout << "\nКниги не найдены.\n";
        }
        else {
            cout << "Найдено " << foundBooks.size() << " книг:\n\n";
            for (const auto& book : foundBooks) {
                cout << "ID: " << book->id << " - \"" << book->title << "\"\n"
                    << "   Автор: " << book->author << "\n"
                    << "   Жанр: " << book->genre << "\n";

                if (!book->annotation.empty()) {
                    string shortAnnotation = book->annotation.substr(0, 100);
                    if (book->annotation.length() > 100) {
                        shortAnnotation += "...";
                    }
                    cout << "   Аннотация: " << shortAnnotation << "\n";
                }

                cout << "   Статус: " << (book->available ? "В наличии" : "Выдана") << "\n\n";
            }
        }

        waitForAnyKey();
    }

    void borrowBook() {
        if (currentUserId == -1) return;

        system("cls");
        cout << "========== ВЗЯТИЕ КНИГИ ==========\n";

        User* user = getCurrentUser();
        if (user == nullptr) {
            cout << "Ошибка: Пользователь не найден!\n";
            Sleep(1500);
            return;
        }

        if (user->borrowedBooks.size() >= 5) {
            cout << "Ошибка: Вы не можете взять более 5 книг одновременно!\n";
            Sleep(1500);
            return;
        }

        cout << "Доступные книги:\n\n";
        bool hasAvailable = false;
        for (const auto& book : books) {
            if (book.available) {
                cout << "ID: " << book.id << " - \"" << book.title << "\" (" << book.author << ")\n";
                hasAvailable = true;
            }
        }

        if (!hasAvailable) {
            cout << "\nВ данный момент нет доступных книг.\n";
            Sleep(1500);
            return;
        }

        vector<string> borrowOptions = {
            "Взять книгу по ID",
            "Посмотреть аннотацию книги",
            "Вернуться в меню"
        };

        Menu borrowMenu("ВЗЯТИЕ КНИГИ", borrowOptions);
        int borrowChoice = borrowMenu.run();

        if (borrowChoice == 2) return;

        if (borrowChoice == 1) {
            cout << "\nВведите ID книги для просмотра аннотации: ";
            int previewId;
            if (!(cin >> previewId)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "\nОшибка: Неверный формат ID!\n";
                waitForAnyKey();
                return;
            }

            auto previewIt = find_if(books.begin(), books.end(),
                [previewId](const Book& b) { return b.id == previewId; });

            if (previewIt == books.end()) {
                cout << "Ошибка: Книга с таким ID не найдена!\n";
                Sleep(1500);
                return;
            }

            cout << "\nАННОТАЦИЯ КНИГИ:\n";
            cout << "========================================\n";
            cout << (previewIt->annotation.empty() ? "Аннотация отсутствует" : previewIt->annotation) << "\n";
            cout << "========================================\n\n";
            cout << "Хотите взять эту книгу? (1 - Да, 0 - Нет): ";
            int takeChoice;
            cin >> takeChoice;

            if (takeChoice != 1) {
                return;
            }

            if (!previewIt->available) {
                cout << "Ошибка: Эта книга уже выдана!\n";
                Sleep(1500);
                return;
            }

            previewIt->available = false;
            user->borrowedBooks.push_back(previewId);

            cout << "\nКнига \"" << previewIt->title << "\" успешно взята!\n";
            Sleep(1500);

            saveBooksData();
            saveUsersData();
            return;
        }

        cout << "\nВведите ID книги для взятия (0 - отмена): ";
        int bookId;
        if (!(cin >> bookId)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\nОшибка: Неверный формат ID!\n";
            waitForAnyKey();
            return;
        }

        if (bookId == 0) return;

        auto it = find_if(books.begin(), books.end(),
            [bookId](const Book& b) { return b.id == bookId; });

        if (it == books.end()) {
            cout << "Ошибка: Книга с таким ID не найдена!\n";
            Sleep(1500);
            return;
        }

        if (!it->available) {
            cout << "Ошибка: Эта книга уже выдана!\n";
            Sleep(1500);
            return;
        }

        it->available = false;
        user->borrowedBooks.push_back(bookId);

        cout << "\nКнига \"" << it->title << "\" успешно взята!\n";
        Sleep(1500);

        saveBooksData();
        saveUsersData();
    }

    void returnBook() {
        if (currentUserId == -1) return;

        system("cls");
        cout << "========== ВОЗВРАТ КНИГИ ==========\n";

        User* user = getCurrentUser();
        if (user == nullptr) {
            cout << "Ошибка: Пользователь не найден!\n";
            Sleep(1500);
            return;
        }

        if (user->borrowedBooks.empty()) {
            cout << "У вас нет взятых книг.\n";
            Sleep(1500);
            return;
        }

        cout << "Ваши взятые книги:\n\n";
        for (int bookId : user->borrowedBooks) {
            auto it = find_if(books.begin(), books.end(),
                [bookId](const Book& b) { return b.id == bookId; });
            if (it != books.end()) {
                cout << "ID: " << bookId << " - \"" << it->title << "\"\n";
            }
        }

        cout << "\nВведите ID книги для возврата (0 - отмена): ";
        int bookId;
        if (!(cin >> bookId)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\nОшибка: Неверный формат ID!\n";
            waitForAnyKey();
            return;
        }

        if (bookId == 0) return;

        auto bookIt = find(user->borrowedBooks.begin(), user->borrowedBooks.end(), bookId);
        if (bookIt == user->borrowedBooks.end()) {
            cout << "Ошибка: У вас нет книги с таким ID!\n";
            Sleep(1500);
            return;
        }

        auto libIt = find_if(books.begin(), books.end(),
            [bookId](const Book& b) { return b.id == bookId; });

        if (libIt != books.end()) {
            libIt->available = true;
            user->borrowedBooks.erase(bookIt);

            cout << "\nКнига \"" << libIt->title << "\" успешно возвращена!\n";
            Sleep(1500);

            saveBooksData();
            saveUsersData();
        }
    }

    void displayUserInfo() {
        if (currentUserId == -1) return;

        system("cls");
        cout << "========== ВАШ ПРОФИЛЬ ==========\n";

        User* user = getCurrentUser();
        if (user == nullptr) {
            cout << "Ошибка: Пользователь не найден!\n";
            waitForAnyKey();
            return;
        }

        cout << "ID: " << user->id << "\n";
        cout << "ФИО: " << user->fullName << "\n";
        if (user->isAdmin) {
            cout << "Роль: Администратор\n";
        }
        else {
            cout << "Роль: Читатель\n";
        }
        cout << "Взято книг: " << user->borrowedBooks.size() << " из 5\n";

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

        waitForAnyKey();
    }

    // ==================== ГЛАВНОЕ МЕНЮ ====================

    void mainMenu() {
        vector<string> options = {
            "Вход в систему",
            "Регистрация",
            "Выход из программы"
        };

        while (true) {
            Menu menu("ГЛАВНОЕ МЕНЮ", options);
            int choice = menu.run();

            switch (choice) {
            case 0:
                if (loginUser()) {
                    User* user = getCurrentUser();
                    if (user != nullptr && user->isAdmin) {
                        adminMenu();
                    }
                    else {
                        userMenu();
                    }
                }
                break;
            case 1:
                registerUser();
                break;
            case 2:
                cout << "\nДо свидания!\n";
                return;
            }
        }
    }

    void adminMenu() {
        User* currentUser = getCurrentUser();
        if (currentUser == nullptr) {
            logout();
            return;
        }

        vector<string> options = {
            "Просмотр каталога книг",
            "Подробная информация о книге",
            "Поиск книг",
            "Добавить книгу",
            "Удалить книгу",
            "Мой профиль",
            "Выход из системы"
        };

        while (true) {
            string title = "МЕНЮ АДМИНИСТРАТОРА | " + currentUser->fullName;

            Menu menu(title, options);
            int choice = menu.run();

            switch (choice) {
            case 0: displayAllBooks(); break;
            case 1: viewBookDetails(); break;
            case 2: searchBooks(); break;
            case 3: addBook(); break;
            case 4: removeBook(); break;
            case 5: displayUserInfo(); break;
            case 6:
                logout();
                return;
            }
        }
    }

    void userMenu() {
        User* currentUser = getCurrentUser();
        if (currentUser == nullptr) {
            logout();
            return;
        }

        vector<string> options = {
            "Просмотр каталога книг",
            "Подробная информация о книге",
            "Поиск книг",
            "Взять книгу",
            "Вернуть книгу",
            "Мой профиль",
            "Выход из системы"
        };

        while (true) {
            string title = "МЕНЮ ЧИТАТЕЛЯ | " + currentUser->fullName;

            Menu menu(title, options);
            int choice = menu.run();

            switch (choice) {
            case 0: displayAllBooks(); break;
            case 1: viewBookDetails(); break;
            case 2: searchBooks(); break;
            case 3: borrowBook(); break;
            case 4: returnBook(); break;
            case 5: displayUserInfo(); break;
            case 6:
                logout();
                return;
            }
        }
    }

    void run() {
        SetConsoleCP(1251);
        SetConsoleOutputCP(1251);

        mainMenu();
    }
};

// ==================== ОСНОВНАЯ ФУНКЦИЯ ====================

int main() {
    LibrarySystem library;
    library.run();
    return 0;
}
