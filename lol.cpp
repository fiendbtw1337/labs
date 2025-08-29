#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <string>
#include <sstream>
#include <stack>
#include <queue>
#include <stdexcept>
#include <memory>
#include <type_traits>
#include <complex>
#include <ctime>
#include <algorithm>

// ==================== ИНТЕРФЕЙСЫ ====================
template <typename T>
class IEnumerable {
public:
    virtual ~IEnumerable() = default;
    virtual std::vector<T> toVector() const = 0;
};

template <typename T>
class ICollection : public IEnumerable<T> {
public:
    virtual void clear() = 0;
    virtual bool isEmpty() const = 0;
    virtual size_t size() const = 0;
    virtual void add(const T& item) = 0;
    virtual bool remove(const T& item) = 0;
    virtual bool contains(const T& item) const = 0;
};

// ==================== ПОЛЬЗОВАТЕЛЬСКИЕ ТИПЫ ====================
using PersonID = int;

class Person {
protected:
    PersonID id;
    std::string firstName;
    std::string middleName;
    std::string lastName;
    std::time_t birthDate;

public:
    Person(PersonID id, const std::string& first, const std::string& middle, 
           const std::string& last, std::time_t birth)
        : id(id), firstName(first), middleName(middle), lastName(last), birthDate(birth) {}

    virtual ~Person() = default;

    PersonID GetID() const { return id; }
    std::string GetFirstName() const { return firstName; }
    std::string GetMiddleName() const { return middleName; }
    std::string GetLastName() const { return lastName; }
    std::time_t GetBirthDate() const { return birthDate; }

    std::string GetFullName() const {
        return firstName + " " + middleName + " " + lastName;
    }

    bool operator<(const Person& other) const { return id < other.id; }
    bool operator>(const Person& other) const { return id > other.id; }
    bool operator==(const Person& other) const { return id == other.id; }

    virtual std::string ToString() const {
        return "Person[ID=" + std::to_string(id) + ", Name=" + GetFullName() + "]";
    }
};

class Student : public Person {
private:
    std::string studentId;
    std::string group;

public:
    Student(PersonID id, const std::string& first, const std::string& middle,
            const std::string& last, std::time_t birth,
            const std::string& studId, const std::string& grp)
        : Person(id, first, middle, last, birth), studentId(studId), group(grp) {}

    std::string GetStudentId() const { return studentId; }
    std::string GetGroup() const { return group; }

    std::string ToString() const override {
        return "Student[ID=" + std::to_string(id) + ", StudentID=" + studentId + 
               ", Group=" + group + ", Name=" + GetFullName() + "]";
    }
};

class Teacher : public Person {
private:
    std::string department;
    std::string position;

public:
    Teacher(PersonID id, const std::string& first, const std::string& middle,
            const std::string& last, std::time_t birth,
            const std::string& dept, const std::string& pos)
        : Person(id, first, middle, last, birth), department(dept), position(pos) {}

    std::string GetDepartment() const { return department; }
    std::string GetPosition() const { return position; }

    std::string ToString() const override {
        return "Teacher[ID=" + std::to_string(id) + ", Department=" + department + 
               ", Position=" + position + ", Name=" + GetFullName() + "]";
    }
};

struct ComplexComparator {
    bool operator()(const std::complex<double>& a, const std::complex<double>& b) const {
        return std::abs(a) < std::abs(b);
    }
};

struct FunctionComparator {
    bool operator()(int (*a)(int), int (*b)(int)) const {
        return reinterpret_cast<uintptr_t>(a) < reinterpret_cast<uintptr_t>(b);
    }
};

// ==================== БИНАРНОЕ ДЕРЕВО ====================
template <typename T, typename Compare = std::less<T>>
struct Node {
    T data;
    Node* left;
    Node* right;

    explicit Node(const T& value) 
        : data(value), left(nullptr), right(nullptr) {}
};

template <typename T, typename Compare>
class TreeIterator {
private:
    std::stack<Node<T, Compare>*> stack;
    bool useInOrder;

    void pushLeft(Node<T, Compare>* node) {
        while (node) {
            stack.push(node);
            node = node->left;
        }
    }

public:
    TreeIterator(Node<T, Compare>* root, bool inOrder = true) : useInOrder(inOrder) {
        if (useInOrder) {
            pushLeft(root);
        } else {
            if (root) stack.push(root);
        }
    }

    T operator*() const {
        if (stack.empty()) throw std::out_of_range("Iterator out of range");
        return stack.top()->data;
    }

    TreeIterator& operator++() {
        if (stack.empty()) return *this;
        
        if (useInOrder) {
            Node<T, Compare>* node = stack.top()->right;
            stack.pop();
            pushLeft(node);
        } else {
            Node<T, Compare>* node = stack.top();
            stack.pop();
            if (node->right) stack.push(node->right);
            if (node->left) stack.push(node->left);
        }
        return *this;
    }

    bool operator!=(const TreeIterator& other) const {
        return !stack.empty() || !other.stack.empty();
    }
};

template <typename T, typename Compare = std::less<T>>
class BinaryTree : public ICollection<T> {
private:
    Node<T, Compare>* root;
    size_t count;
    Compare comp;

    void clearRecursive(Node<T, Compare>* node) {
        if (node) {
            clearRecursive(node->left);
            clearRecursive(node->right);
            delete node;
        }
    }

    Node<T, Compare>* insertRecursive(Node<T, Compare>* node, const T& value) {
        if (!node) {
            count++;
            return new Node<T, Compare>(value);
        }
        
        if (comp(value, node->data)) {
            node->left = insertRecursive(node->left, value);
        } else if (comp(node->data, value)) {
            node->right = insertRecursive(node->right, value);
        }
        return node;
    }

    Node<T, Compare>* findRecursive(Node<T, Compare>* node, const T& value) const {
        if (!node) return nullptr;
        if (value == node->data) return node;
        if (comp(value, node->data)) return findRecursive(node->left, value);
        return findRecursive(node->right, value);
    }

    Node<T, Compare>* findMin(Node<T, Compare>* node) const {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }

    Node<T, Compare>* removeRecursive(Node<T, Compare>* node, const T& value) {
        if (!node) return nullptr;

        if (comp(value, node->data)) {
            node->left = removeRecursive(node->left, value);
        } else if (comp(node->data, value)) {
            node->right = removeRecursive(node->right, value);
        } else {
            if (!node->left && !node->right) {
                delete node;
                count--;
                return nullptr;
            } else if (!node->left) {
                Node<T, Compare>* temp = node->right;
                delete node;
                count--;
                return temp;
            } else if (!node->right) {
                Node<T, Compare>* temp = node->left;
                delete node;
                count--;
                return temp;
            } else {
                Node<T, Compare>* successor = findMin(node->right);
                node->data = successor->data;
                node->right = removeRecursive(node->right, successor->data);
            }
        }
        return node;
    }

    // Обходы
    void inOrderRecursive(Node<T, Compare>* node, std::vector<T>& result) const {
        if (node) {
            inOrderRecursive(node->left, result);
            result.push_back(node->data);
            inOrderRecursive(node->right, result);
        }
    }

    void preOrderRecursive(Node<T, Compare>* node, std::vector<T>& result) const {
        if (node) {
            result.push_back(node->data);
            preOrderRecursive(node->left, result);
            preOrderRecursive(node->right, result);
        }
    }

    void postOrderRecursive(Node<T, Compare>* node, std::vector<T>& result) const {
        if (node) {
            postOrderRecursive(node->left, result);
            postOrderRecursive(node->right, result);
            result.push_back(node->data);
        }
    }

    // Копирование дерева
    Node<T, Compare>* copyTree(Node<T, Compare>* node) const {
        if (!node) return nullptr;
        Node<T, Compare>* newNode = new Node<T, Compare>(node->data);
        newNode->left = copyTree(node->left);
        newNode->right = copyTree(node->right);
        return newNode;
    }

    // Для map и where
    template <typename U, typename CompareU>
    void mapRecursive(Node<T, Compare>* node, BinaryTree<U, CompareU>& resultTree, std::function<U(T)> func) const {
        if (!node) return;
        resultTree.insert(func(node->data));
        mapRecursive(node->left, resultTree, func);
        mapRecursive(node->right, resultTree, func);
    }

    void whereRecursive(Node<T, Compare>* node, BinaryTree<T, Compare>& resultTree, std::function<bool(T)> predicate) const {
        if (!node) return;
        if (predicate(node->data)) {
            resultTree.insert(node->data);
        }
        whereRecursive(node->left, resultTree, predicate);
        whereRecursive(node->right, resultTree, predicate);
    }

    // Для сериализации
    void serializeRecursive(Node<T, Compare>* node, std::stringstream& ss, const std::string& format) const {
        if (!node) {
            ss << "# "; // маркер пустого узла
            return;
        }
        
        if (format == "preorder") {
            ss << node->data << " ";
            serializeRecursive(node->left, ss, format);
            serializeRecursive(node->right, ss, format);
        } else if (format == "inorder") {
            serializeRecursive(node->left, ss, format);
            ss << node->data << " ";
            serializeRecursive(node->right, ss, format);
        } else if (format == "postorder") {
            serializeRecursive(node->left, ss, format);
            serializeRecursive(node->right, ss, format);
            ss << node->data << " ";
        }
    }

    Node<T, Compare>* deserializeRecursive(std::stringstream& ss, const std::string& format) {
        std::string token;
        if (!(ss >> token) || token == "#") return nullptr;

        T value;
        std::istringstream tokenStream(token);
        tokenStream >> value;

        Node<T, Compare>* node = new Node<T, Compare>(value);
        
        if (format == "preorder") {
            node->left = deserializeRecursive(ss, format);
            node->right = deserializeRecursive(ss, format);
        }
        return node;
    }

    // Для извлечения поддерева
    Node<T, Compare>* extractSubtreeRecursive(Node<T, Compare>* node, const T& value) const {
        if (!node) return nullptr;
        if (node->data == value) {
            return copyTree(node);
        }
        
        Node<T, Compare>* leftResult = extractSubtreeRecursive(node->left, value);
        if (leftResult) return leftResult;
        
        return extractSubtreeRecursive(node->right, value);
    }

    // Для проверки вхождения поддерева
    bool isSubtreeRecursive(Node<T, Compare>* mainTree, Node<T, Compare>* subTree) const {
        if (!subTree) return true;
        if (!mainTree) return false;
        
        if (mainTree->data == subTree->data) {
            return isSubtreeRecursive(mainTree->left, subTree->left) &&
                   isSubtreeRecursive(mainTree->right, subTree->right);
        }
        
        return isSubtreeRecursive(mainTree->left, subTree) ||
               isSubtreeRecursive(mainTree->right, subTree);
    }

    // Для поиска по пути
    Node<T, Compare>* findNodeByPathRecursive(Node<T, Compare>* node, const std::vector<std::string>& path, size_t index) const {
        if (!node || index >= path.size()) return node;
        
        if (path[index] == "L") {
            return findNodeByPathRecursive(node->left, path, index + 1);
        } else if (path[index] == "R") {
            return findNodeByPathRecursive(node->right, path, index + 1);
        }
        
        return nullptr;
    }

    // Для слияния деревьев
    void mergeTreeRecursive(Node<T, Compare>* otherNode) {
        if (!otherNode) return;
        insert(otherNode->data);
        mergeTreeRecursive(otherNode->left);
        mergeTreeRecursive(otherNode->right);
    }

public:
    BinaryTree() : root(nullptr), count(0) {}
    
    BinaryTree(const BinaryTree& other) : count(other.count) {
        root = copyTree(other.root);
    }
    
    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            clear();
            root = copyTree(other.root);
            count = other.count;
        }
        return *this;
    }
    
    ~BinaryTree() {
        clear();
    }

    // ICollection implementation
    void clear() override {
        clearRecursive(root);
        root = nullptr;
        count = 0;
    }

    bool isEmpty() const override {
        return root == nullptr;
    }

    size_t size() const override {
        return count;
    }

    void add(const T& item) override {
        insert(item);
    }

    bool remove(const T& item) override {
        size_t oldCount = count;
        root = removeRecursive(root, item);
        return count < oldCount;
    }

    bool contains(const T& item) const override {
        return find(item);
    }

    std::vector<T> toVector() const override {
        return inOrder();
    }

    // Основные методы
    void insert(const T& value) {
        root = insertRecursive(root, value);
    }

    bool find(const T& value) const {
        return findRecursive(root, value) != nullptr;
    }

    // Обходы
    std::vector<T> inOrder() const {
        std::vector<T> result;
        inOrderRecursive(root, result);
        return result;
    }

    std::vector<T> preOrder() const {
        std::vector<T> result;
        preOrderRecursive(root, result);
        return result;
    }

    std::vector<T> postOrder() const {
        std::vector<T> result;
        postOrderRecursive(root, result);
        return result;
    }

    // Map и Where
    template <typename U, typename CompareU = std::less<U>>
    BinaryTree<U, CompareU> map(std::function<U(T)> func) const {
        BinaryTree<U, CompareU> result;
        mapRecursive(root, result, func);
        return result;
    }

    BinaryTree<T, Compare> where(std::function<bool(T)> predicate) const {
        BinaryTree<T, Compare> result;
        whereRecursive(root, result, predicate);
        return result;
    }

    // Reduce
    template <typename U>
    U reduce(std::function<U(U, T)> func, U initial) const {
        U result = initial;
        for (const auto& item : *this) {
            result = func(result, item);
        }
        return result;
    }

    // СЛИЯНИЕ ДЕРЕВЬЕВ
    void merge(const BinaryTree<T, Compare>& other) {
        mergeTreeRecursive(other.root);
    }

    // ИЗВЛЕЧЕНИЕ ПОДДЕРЕВА
    BinaryTree<T, Compare> extractSubtree(const T& value) const {
        BinaryTree<T, Compare> result;
        result.root = extractSubtreeRecursive(root, value);
        return result;
    }

    // ПОИСК НА ВХОЖДЕНИЕ ПОДДЕРЕва
    bool containsSubtree(const BinaryTree<T, Compare>& subtree) const {
        return isSubtreeRecursive(root, subtree.root);
    }

    // СОХРАНЕНИЕ В СТРОКУ
    std::string serialize(const std::string& format = "preorder") const {
        std::stringstream ss;
        serializeRecursive(root, ss, format);
        return ss.str();
    }

    // ЧТЕНИЕ ИЗ СТРОКИ
    void deserialize(const std::string& data, const std::string& format = "preorder") {
        clear();
        std::stringstream ss(data);
        root = deserializeRecursive(ss, format);
    }

    // ПОИСК УЗЛА ПО ПУТИ
    T findNodeByPath(const std::vector<std::string>& path) const {
        Node<T, Compare>* node = findNodeByPathRecursive(root, path, 0);
        if (!node) throw std::runtime_error("Node not found at specified path");
        return node->data;
    }

    // ПОИСК ПО ОТНОСИТЕЛЬНОМУ ПУТИ
    T findNodeByRelativePath(const T& startValue, const std::vector<std::string>& path) const {
        Node<T, Compare>* startNode = findRecursive(root, startValue);
        if (!startNode) throw std::runtime_error("Start node not found");
        
        Node<T, Compare>* resultNode = findNodeByPathRecursive(startNode, path, 0);
        if (!resultNode) throw std::runtime_error("Node not found at relative path");
        return resultNode->data;
    }

    // Итераторы
    TreeIterator<T, Compare> begin() const {
        return TreeIterator<T, Compare>(root, true);
    }

    TreeIterator<T, Compare> end() const {
        return TreeIterator<T, Compare>(nullptr, true);
    }

    // Вывод
    void print(const std::string& traversal = "inorder") const {
        if (isEmpty()) {
            std::cout << "Дерево пусто" << std::endl;
            return;
        }

        std::vector<T> elements;
        if (traversal == "preorder") {
            elements = preOrder();
            std::cout << "Прямой обход: ";
        } else if (traversal == "postorder") {
            elements = postOrder();
            std::cout << "Обратный обход: ";
        } else {
            elements = inOrder();
            std::cout << "Симметричный обход: ";
        }

        for (const auto& item : elements) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};

// ==================== ТЕСТОВЫЕ ФУНКЦИИ ====================
int increment1(int x) { return x + 1; }
int increment2(int x) { return x + 2; }
int square(int x) { return x * x; }
double doubleSquare(double x) { return x * x; }

void displayMenu() {
    std::cout << "\n=== МЕНЮ БИНАРНОГО ДЕРЕВА ===" << std::endl;
    std::cout << "1. Тестирование целых чисел" << std::endl;
    std::cout << "2. Тестирование вещественных чисел" << std::endl;
    std::cout << "3. Тестирование комплексных чисел" << std::endl;
    std::cout << "4. Тестирование строк" << std::endl;
    std::cout << "5. Тестирование функций" << std::endl;
    std::cout << "6. Тестирование студентов" << std::endl;
    std::cout << "7. Тестирование преподавателей" << std::endl;
    std::cout << "8. Тестирование всех операций" << std::endl;
    std::cout << "0. Выход" << std::endl;
    std::cout << "Выберите опцию: ";
}

void testIntegers() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ ЦЕЛЫХ ЧИСЕЛ ===" << std::endl;
    
    BinaryTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);
    
    std::cout << "Исходное дерево: ";
    tree.print();

    std::cout << "Поиск 40: " << (tree.find(40) ? "Найден" : "Не найден") << std::endl;
    std::cout << "Поиск 100: " << (tree.find(100) ? "Найден" : "Не найден") << std::endl;

    std::cout << "MAP (удвоение): ";
    auto doubled = tree.map<int>([](int x) { return x * 2; });
    doubled.print();

    std::cout << "WHERE (четные): ";
    auto evens = tree.where([](int x) { return x % 2 == 0; });
    evens.print();

    std::cout << "REDUCE (сумма): " << tree.reduce<int>([](int acc, int x) { return acc + x; }, 0) << std::endl;
}

void testDoubles() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ ВЕЩЕСТВЕННЫХ ЧИСЕЛ ===" << std::endl;
    
    BinaryTree<double> tree;
    tree.insert(3.14);
    tree.insert(2.71);
    tree.insert(1.41);
    tree.insert(1.73);
    tree.insert(0.577);
    
    std::cout << "Исходное дерево: ";
    tree.print();

    std::cout << "MAP (квадраты): ";
    auto squared = tree.map<double>([](double x) { return x * x; });
    squared.print();

    std::cout << "WHERE (>2.0): ";
    auto large = tree.where([](double x) { return x > 2.0; });
    large.print();

    std::cout << "REDUCE (произведение): " << tree.reduce<double>([](double acc, double x) { return acc * x; }, 1.0) << std::endl;
}

void testComplexNumbers() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ КОМПЛЕКСНЫХ ЧИСЕЛ ===" << std::endl;
    
    BinaryTree<std::complex<double>, ComplexComparator> tree;
    tree.insert(std::complex<double>(1, 2));
    tree.insert(std::complex<double>(3, 4));
    tree.insert(std::complex<double>(0, 1));
    tree.insert(std::complex<double>(2, 0));
    
    std::cout << "Комплексные числа: ";
    for (const auto& item : tree) {
        std::cout << "(" << item.real() << "+" << item.imag() << "i) ";
    }
    std::cout << std::endl;

    std::cout << "WHERE (real > 1): ";
    auto filtered = tree.where([](const std::complex<double>& c) { return c.real() > 1; });
    for (const auto& item : filtered) {
        std::cout << "(" << item.real() << "+" << item.imag() << "i) ";
    }
    std::cout << std::endl;
}

void testStrings() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ СТРОК ===" << std::endl;
    
    BinaryTree<std::string> tree;
    tree.insert("apple");
    tree.insert("banana");
    tree.insert("cherry");
    tree.insert("date");
    tree.insert("elderberry");
    
    std::cout << "Исходное дерево: ";
    tree.print();

    std::cout << "MAP (верхний регистр): ";
    auto upper = tree.map<std::string>([](const std::string& s) {
        std::string result = s;
        for (char& c : result) c = std::toupper(c);
        return result;
    });
    upper.print();

    std::cout << "WHERE (длина > 5): ";
    auto longStrings = tree.where([](const std::string& s) { return s.length() > 5; });
    longStrings.print();
}

void testFunctions() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ ФУНКЦИЙ ===" << std::endl;
    
    BinaryTree<int(*)(int), FunctionComparator> tree;
    tree.insert(&increment1);
    tree.insert(&increment2);
    tree.insert(&square);
    
    std::cout << "Функции в дереве: " << std::endl;
    for (const auto& func : tree) {
        std::cout << "f(5) = " << func(5) << std::endl;
    }

    std::cout << "WHERE (f(0) > 1): " << std::endl;
    auto filtered = tree.where([](int (*func)(int)) { return func(0) > 1; });
    for (const auto& func : filtered) {
        std::cout << "f(0) = " << func(0) << std::endl;
    }
}

void testStudents() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ СТУДЕНТОВ ===" << std::endl;
    
    BinaryTree<Student> tree;
    
    Student s1(1, "Иван", "Иванович", "Иванов", 0, "ST001", "Группа 101");
    Student s2(2, "Петр", "Петрович", "Петров", 0, "ST002", "Группа 102");
    Student s3(3, "Анна", "Сергеевна", "Сидорова", 0, "ST003", "Группа 101");
    
    tree.insert(s1);
    tree.insert(s2);
    tree.insert(s3);
    
    std::cout << "Все студенты: " << std::endl;
    for (const auto& student : tree) {
        std::cout << "  " << student.ToString() << std::endl;
    }

    std::cout << "WHERE (Группа 101): " << std::endl;
    auto group101 = tree.where([](const Student& s) { return s.GetGroup() == "Группа 101"; });
    for (const auto& student : group101) {
        std::cout << "  " << student.ToString() << std::endl;
    }
}

void testTeachers() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ ПРЕПОДАВАТЕЛЕЙ ===" << std::endl;
    
    BinaryTree<Teacher> tree;
    
    Teacher t1(1, "Мария", "Ивановна", "Смирнова", 0, "ИТ", "Доцент");
    Teacher t2(2, "Алексей", "Владимирович", "Кузнецов", 0, "Математика", "Профессор");
    Teacher t3(3, "Ольга", "Петровна", "Васильева", 0, "Физика", "Старший преподаватель");
    
    tree.insert(t1);
    tree.insert(t2);
    tree.insert(t3);
    
    std::cout << "Все преподаватели: " << std::endl;
    for (const auto& teacher : tree) {
        std::cout << "  " << teacher.ToString() << std::endl;
    }

    std::cout << "WHERE (Профессор): " << std::endl;
    auto professors = tree.where([](const Teacher& t) { return t.GetPosition() == "Профессор"; });
    for (const auto& teacher : professors) {
        std::cout << "  " << teacher.ToString() << std::endl;
    }
}

void testAllOperations() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ ВСЕХ ОПЕРАЦИЙ ===" << std::endl;
    
    BinaryTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);
    
    std::cout << "Исходное дерево: ";
    tree.print();

    // Все операции
    std::cout << "1. MAP: ";
    tree.map<int>([](int x) { return x * 2; }).print();

    std::cout << "2. WHERE: ";
    tree.where([](int x) { return x % 2 == 0; }).print();

    std::cout << "3. СЛИЯНИЕ: ";
    BinaryTree<int> other;
    other.insert(35);
    other.insert(45);
    tree.merge(other);
    tree.print();

    std::cout << "4. ПОДДЕРЕВО: ";
    tree.extractSubtree(30).print();

    std::cout << "5. ПОИСК ПОДДЕРЕВА: " << tree.containsSubtree(tree.extractSubtree(30)) << std::endl;
    std::cout << "6. ПОИСК ЭЛЕМЕНТА 40: " << tree.find(40) << std::endl;

    std::cout << "7. СЕРИАЛИЗАЦИЯ: " << tree.serialize() << std::endl;
}

void runTests() {
    int choice;
    do {
        displayMenu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1: testIntegers(); break;
            case 2: testDoubles(); break;
            case 3: testComplexNumbers(); break;
            case 4: testStrings(); break;
            case 5: testFunctions(); break;
            case 6: testStudents(); break;
            case 7: testTeachers(); break;
            case 8: testAllOperations(); break;
            case 0: std::cout << "Выход..." << std::endl; break;
            default: std::cout << "Неверный выбор!" << std::endl;
        }
    } while (choice != 0);
}

int main() {
    std::cout << "=== БИНАРНОЕ ДЕРЕВО С ПОДДЕРЖКОЙ РАЗЛИЧНЫХ ТИПОВ ===" << std::endl;
    
    runTests();

    std::cout << "\n=== ТЕСТИРОВАНИЕ ЗАВЕРШЕНО ===" << std::endl;
    return 0;
}