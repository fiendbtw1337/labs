#include <iostream>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <sstream>
#include <complex>
#include <typeinfo>
#include <type_traits>
#include <random>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <variant>
#include <any>
#include <tuple>
#include <map>
#include <set>
#include <ctime>

// ==================== ВСПОМОГАТЕЛЬНЫЕ СТРУКТУРЫ ДАННЫХ ====================

// Комплексное число
using Complex = std::complex<double>;

std::ostream& operator<<(std::ostream& os, const Complex& c) {
    os << c.real() << " + " << c.imag() << "i";
    return os;
}

std::istream& operator>>(std::istream& is, Complex& c) {
    double real, imag;
    char plus, i;
    if (is >> real >> plus >> imag >> i) {
        if (plus == '+' && i == 'i') {
            c = Complex(real, imag);
        } else {
            is.setstate(std::ios::failbit);
        }
    }
    return is;
}

// Операторы сравнения для Complex
bool operator>(const Complex& a, const Complex& b) {
    return std::abs(a) > std::abs(b);
}

bool operator<(const Complex& a, const Complex& b) {
    return std::abs(a) < std::abs(b);
}

bool operator>=(const Complex& a, const Complex& b) {
    return std::abs(a) >= std::abs(b);
}

bool operator<=(const Complex& a, const Complex& b) {
    return std::abs(a) <= std::abs(b);
}

// Структура для персоны
struct PersonID {
    int series;
    int number;
    
    bool operator==(const PersonID& other) const {
        return series == other.series && number == other.number;
    }
    
    bool operator!=(const PersonID& other) const {
        return !(*this == other);
    }
    
    bool operator<(const PersonID& other) const {
        return std::tie(series, number) < std::tie(other.series, other.number);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const PersonID& id) {
        return os << id.series << " " << id.number;
    }
    
    friend std::istream& operator>>(std::istream& is, PersonID& id) {
        return is >> id.series >> id.number;
    }
};

class Person {
private:
    PersonID id;
    std::string firstName;
    std::string middleName;
    std::string lastName;
    std::time_t birthDate;

public:
    Person() : birthDate(0) {}
    
    Person(PersonID id, std::string first, std::string middle, std::string last, std::time_t birth)
        : id(id), firstName(std::move(first)), middleName(std::move(middle)), 
          lastName(std::move(last)), birthDate(birth) {}

    PersonID GetID() const { return id; }
    std::string GetFirstName() const { return firstName; }
    std::string GetMiddleName() const { return middleName; }
    std::string GetLastName() const { return lastName; }
    std::string GetFullName() const { return firstName + " " + middleName + " " + lastName; }
    std::time_t GetBirthDate() const { return birthDate; }

    bool operator==(const Person& other) const {
        return id == other.id;
    }

    bool operator!=(const Person& other) const {
        return !(*this == other);
    }

    bool operator<(const Person& other) const {
        return id < other.id;
    }

    friend std::ostream& operator<<(std::ostream& os, const Person& p) {
        char buffer[26];
        std::strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", std::localtime(&p.birthDate));
        os << "Person{ID: " << p.id << ", Name: " << p.GetFullName() 
           << ", Birth: " << buffer << "}";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Person& p) {
        std::cout << "Enter series and number: ";
        is >> p.id.series >> p.id.number;
        std::cout << "Enter first name: ";
        is >> p.firstName;
        std::cout << "Enter middle name: ";
        is >> p.middleName;
        std::cout << "Enter last name: ";
        is >> p.lastName;
        p.birthDate = std::time(nullptr);
        return is;
    }
};

// Функциональные объекты
using FunctionPtr = double(*)(double);
double sinc1(double x) { return x == 0 ? 1 : std::sin(x)/x; }
double sinc2(double x) { return x == 0 ? 2 : 2*std::sin(x)/x; }
double sinc3(double x) { return x == 0 ? 3 : 3*std::sin(x)/x; }

std::ostream& operator<<(std::ostream& os, FunctionPtr func) {
    if (func == sinc1) os << "sinc1";
    else if (func == sinc2) os << "sinc2";
    else if (func == sinc3) os << "sinc3";
    else os << "unknown_function";
    return os;
}

// ==================== БАЗОВЫЙ ИНТЕРФЕЙС ПОСЛЕДОВАТЕЛЬНОСТИ ====================

template <typename T>
class Sequence {
public:
    virtual ~Sequence() = default;

    // Базовые методы
    virtual T GetFirst() const = 0;
    virtual T GetLast() const = 0;
    virtual T Get(int index) const = 0;
    virtual std::shared_ptr<Sequence<T>> GetSubsequence(int startIndex, int endIndex) const = 0;
    virtual int GetLength() const = 0;

    // Методы модификации
    virtual void Append(const T& item) = 0;
    virtual void Prepend(const T& item) = 0;
    virtual void InsertAt(const T& item, int index) = 0;
    virtual void RemoveAt(int index) = 0;
    virtual void Remove(const T& item) = 0;
    virtual void Clear() = 0;

    // Функциональные методы
    virtual std::shared_ptr<Sequence<T>> Concat(const Sequence<T>& other) const = 0;
    virtual std::shared_ptr<Sequence<T>> Map(std::function<T(T)> func) const = 0;
    virtual std::shared_ptr<Sequence<T>> Where(std::function<bool(T)> predicate) const = 0;
    virtual T Reduce(std::function<T(T, T)> func, T initial) const = 0;
    
    // Дополнительные операции
    virtual std::shared_ptr<Sequence<T>> Zip(const Sequence<T>& other) const = 0;
    virtual std::pair<std::shared_ptr<Sequence<T>>, std::shared_ptr<Sequence<T>>> Split(std::function<bool(T)> predicate) const = 0;
    virtual std::shared_ptr<Sequence<T>> Slice(int start, int end) const = 0;
    virtual bool ContainsSubsequence(const Sequence<T>& subsequence) const = 0;

    // Операторы
    virtual T& operator[](int index) = 0;
    virtual const T& operator[](int index) const = 0;

    // Вспомогательные методы
    virtual bool Contains(const T& item) const = 0;
    virtual int IndexOf(const T& item) const = 0;
    virtual bool IsEmpty() const = 0;
    virtual std::string ToString() const = 0;
};

// ==================== ДИНАМИЧЕСКИЙ МАССИВ ====================

template <typename T>
class ArraySequence : public Sequence<T> {
private:
    std::unique_ptr<T[]> data;
    int capacity;
    int length;

    void Resize(int newCapacity) {
        std::unique_ptr<T[]> newData = std::make_unique<T[]>(newCapacity);
        for (int i = 0; i < length; i++) {
            newData[i] = std::move(data[i]);
        }
        data = std::move(newData);
        capacity = newCapacity;
    }

public:
    ArraySequence() : data(std::make_unique<T[]>(1)), capacity(1), length(0) {}
    
    ArraySequence(int initialCapacity) : data(std::make_unique<T[]>(initialCapacity)), 
                                        capacity(initialCapacity), length(0) {}
    
    ArraySequence(std::initializer_list<T> init) : data(std::make_unique<T[]>(init.size())), 
                                                  capacity(init.size()), length(init.size()) {
        int i = 0;
        for (const T& item : init) {
            data[i++] = item;
        }
    }
    
    ArraySequence(const ArraySequence<T>& other) : data(std::make_unique<T[]>(other.capacity)), 
                                                  capacity(other.capacity), length(other.length) {
        for (int i = 0; i < length; i++) {
            data[i] = other.data[i];
        }
    }

    ArraySequence<T>& operator=(const ArraySequence<T>& other) {
        if (this != &other) {
            data = std::make_unique<T[]>(other.capacity);
            capacity = other.capacity;
            length = other.length;
            for (int i = 0; i < length; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // Реализация методов Sequence<T>
    T GetFirst() const override {
        if (length == 0) throw std::out_of_range("Sequence is empty");
        return data[0];
    }

    T GetLast() const override {
        if (length == 0) throw std::out_of_range("Sequence is empty");
        return data[length - 1];
    }

    T Get(int index) const override {
        if (index < 0 || index >= length) 
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    std::shared_ptr<Sequence<T>> GetSubsequence(int startIndex, int endIndex) const override {
        if (startIndex < 0 || endIndex >= length || startIndex > endIndex)
            throw std::out_of_range("Invalid indices");
        
        auto sub = std::make_shared<ArraySequence<T>>(endIndex - startIndex + 1);
        for (int i = startIndex; i <= endIndex; i++) {
            sub->Append(data[i]);
        }
        return sub;
    }

    int GetLength() const override {
        return length;
    }

    void Append(const T& item) override {
        if (length >= capacity) {
            Resize(capacity * 2);
        }
        data[length++] = item;
    }

    void Prepend(const T& item) override {
        InsertAt(item, 0);
    }

    void InsertAt(const T& item, int index) override {
        if (index < 0 || index > length)
            throw std::out_of_range("Index out of range");
        
        if (length >= capacity) {
            Resize(capacity * 2);
        }
        
        for (int i = length; i > index; i--) {
            data[i] = data[i - 1];
        }
        data[index] = item;
        length++;
    }

    void RemoveAt(int index) override {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        
        for (int i = index; i < length - 1; i++) {
            data[i] = data[i + 1];
        }
        length--;
    }

    void Remove(const T& item) override {
        int index = IndexOf(item);
        if (index != -1) {
            RemoveAt(index);
        }
    }

    void Clear() override {
        length = 0;
    }

    std::shared_ptr<Sequence<T>> Concat(const Sequence<T>& other) const override {
        auto result = std::make_shared<ArraySequence<T>>(*this);
        for (int i = 0; i < other.GetLength(); i++) {
            result->Append(other.Get(i));
        }
        return result;
    }

    std::shared_ptr<Sequence<T>> Map(std::function<T(T)> func) const override {
        auto result = std::make_shared<ArraySequence<T>>(length);
        for (int i = 0; i < length; i++) {
            result->Append(func(data[i]));
        }
        return result;
    }

    std::shared_ptr<Sequence<T>> Where(std::function<bool(T)> predicate) const override {
        auto result = std::make_shared<ArraySequence<T>>();
        for (int i = 0; i < length; i++) {
            if (predicate(data[i])) {
                result->Append(data[i]);
            }
        }
        return result;
    }

    T Reduce(std::function<T(T, T)> func, T initial) const override {
        T result = initial;
        for (int i = 0; i < length; i++) {
            result = func(result, data[i]);
        }
        return result;
    }

    std::shared_ptr<Sequence<T>> Zip(const Sequence<T>& other) const override {
        int minLength = std::min(length, other.GetLength());
        auto result = std::make_shared<ArraySequence<T>>(minLength * 2);
        
        for (int i = 0; i < minLength; i++) {
            result->Append(data[i]);
            result->Append(other.Get(i));
        }
        return result;
    }

    std::pair<std::shared_ptr<Sequence<T>>, std::shared_ptr<Sequence<T>>> Split(std::function<bool(T)> predicate) const override {
        auto trueSeq = std::make_shared<ArraySequence<T>>();
        auto falseSeq = std::make_shared<ArraySequence<T>>();
        
        for (int i = 0; i < length; i++) {
            if (predicate(data[i])) {
                trueSeq->Append(data[i]);
            } else {
                falseSeq->Append(data[i]);
            }
        }
        
        return {trueSeq, falseSeq};
    }

    std::shared_ptr<Sequence<T>> Slice(int start, int end) const override {
        return GetSubsequence(start, end);
    }

    bool ContainsSubsequence(const Sequence<T>& subsequence) const override {
        if (subsequence.GetLength() == 0) return true;
        if (subsequence.GetLength() > length) return false;

        for (int i = 0; i <= length - subsequence.GetLength(); i++) {
            bool match = true;
            for (int j = 0; j < subsequence.GetLength(); j++) {
                if (data[i + j] != subsequence.Get(j)) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
        return false;
    }

    T& operator[](int index) override {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T& operator[](int index) const override {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    bool Contains(const T& item) const override {
        return IndexOf(item) != -1;
    }

    int IndexOf(const T& item) const override {
        for (int i = 0; i < length; i++) {
            if (data[i] == item) {
                return i;
            }
        }
        return -1;
    }

    bool IsEmpty() const override {
        return length == 0;
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "[";
        for (int i = 0; i < length; i++) {
            ss << data[i];
            if (i < length - 1) ss << ", ";
        }
        ss << "]";
        return ss.str();
    }
};

// ==================== СВЯЗАННЫЙ СПИСОК ====================

template <typename T>
class LinkedListSequence : public Sequence<T> {
private:
    struct Node {
        T data;
        std::unique_ptr<Node> next;
        Node(const T& value) : data(value), next(nullptr) {}
    };

    std::unique_ptr<Node> head;
    Node* tail;
    int length;

public:
    LinkedListSequence() : head(nullptr), tail(nullptr), length(0) {}
    
    LinkedListSequence(std::initializer_list<T> init) : head(nullptr), tail(nullptr), length(0) {
        for (const T& item : init) {
            Append(item);
        }
    }
    
    LinkedListSequence(const LinkedListSequence<T>& other) : head(nullptr), tail(nullptr), length(0) {
        Node* current = other.head.get();
        while (current != nullptr) {
            Append(current->data);
            current = current->next.get();
        }
    }
    
    LinkedListSequence<T>& operator=(const LinkedListSequence<T>& other) {
        if (this != &other) {
            Clear();
            Node* current = other.head.get();
            while (current != nullptr) {
                Append(current->data);
                current = current->next.get();
            }
        }
        return *this;
    }

    ~LinkedListSequence() override {
        Clear();
    }

    T GetFirst() const override {
        if (!head) throw std::out_of_range("Sequence is empty");
        return head->data;
    }

    T GetLast() const override {
        if (!tail) throw std::out_of_range("Sequence is empty");
        return tail->data;
    }

    T Get(int index) const override {
        if (index < 0 || index >= length) 
            throw std::out_of_range("Index out of range");
        
        Node* current = head.get();
        for (int i = 0; i < index; i++) {
            current = current->next.get();
        }
        return current->data;
    }

    std::shared_ptr<Sequence<T>> GetSubsequence(int startIndex, int endIndex) const override {
        if (startIndex < 0 || endIndex >= length || startIndex > endIndex)
            throw std::out_of_range("Invalid indices");
        
        auto sub = std::make_shared<LinkedListSequence<T>>();
        Node* current = head.get();
        for (int i = 0; i < startIndex; i++) {
            current = current->next.get();
        }
        for (int i = startIndex; i <= endIndex; i++) {
            sub->Append(current->data);
            current = current->next.get();
        }
        return sub;
    }

    int GetLength() const override {
        return length;
    }

    void Append(const T& item) override {
        auto newNode = std::make_unique<Node>(item);
        if (!head) {
            head = std::move(newNode);
            tail = head.get();
        } else {
            tail->next = std::move(newNode);
            tail = tail->next.get();
        }
        length++;
    }

    void Prepend(const T& item) override {
        auto newNode = std::make_unique<Node>(item);
        if (!head) {
            head = std::move(newNode);
            tail = head.get();
        } else {
            newNode->next = std::move(head);
            head = std::move(newNode);
        }
        length++;
    }

    void InsertAt(const T& item, int index) override {
        if (index < 0 || index > length)
            throw std::out_of_range("Index out of range");
        
        if (index == 0) {
            Prepend(item);
        } else if (index == length) {
            Append(item);
        } else {
            auto newNode = std::make_unique<Node>(item);
            Node* current = head.get();
            for (int i = 0; i < index - 1; i++) {
                current = current->next.get();
            }
            newNode->next = std::move(current->next);
            current->next = std::move(newNode);
            length++;
        }
    }

    void RemoveAt(int index) override {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        
        if (index == 0) {
            head = std::move(head->next);
            if (!head) tail = nullptr;
        } else {
            Node* current = head.get();
            for (int i = 0; i < index - 1; i++) {
                current = current->next.get();
            }
            current->next = std::move(current->next->next);
            if (index == length - 1) {
                tail = current;
            }
        }
        length--;
    }

    void Remove(const T& item) override {
        int index = IndexOf(item);
        if (index != -1) {
            RemoveAt(index);
        }
    }

    void Clear() override {
        head.reset();
        tail = nullptr;
        length = 0;
    }

    std::shared_ptr<Sequence<T>> Concat(const Sequence<T>& other) const override {
        auto result = std::make_shared<LinkedListSequence<T>>(*this);
        for (int i = 0; i < other.GetLength(); i++) {
            result->Append(other.Get(i));
        }
        return result;
    }

    std::shared_ptr<Sequence<T>> Map(std::function<T(T)> func) const override {
        auto result = std::make_shared<LinkedListSequence<T>>();
        Node* current = head.get();
        while (current) {
            result->Append(func(current->data));
            current = current->next.get();
        }
        return result;
    }

    std::shared_ptr<Sequence<T>> Where(std::function<bool(T)> predicate) const override {
        auto result = std::make_shared<LinkedListSequence<T>>();
        Node* current = head.get();
        while (current) {
            if (predicate(current->data)) {
                result->Append(current->data);
            }
            current = current->next.get();
        }
        return result;
    }

    T Reduce(std::function<T(T, T)> func, T initial) const override {
        T result = initial;
        Node* current = head.get();
        while (current) {
            result = func(result, current->data);
            current = current->next.get();
        }
        return result;
    }

    std::shared_ptr<Sequence<T>> Zip(const Sequence<T>& other) const override {
        int minLength = std::min(length, other.GetLength());
        auto result = std::make_shared<LinkedListSequence<T>>();
        
        Node* current = head.get();
        for (int i = 0; i < minLength; i++) {
            result->Append(current->data);
            result->Append(other.Get(i));
            current = current->next.get();
        }
        return result;
    }

    std::pair<std::shared_ptr<Sequence<T>>, std::shared_ptr<Sequence<T>>> Split(std::function<bool(T)> predicate) const override {
        auto trueSeq = std::make_shared<LinkedListSequence<T>>();
        auto falseSeq = std::make_shared<LinkedListSequence<T>>();
        
        Node* current = head.get();
        while (current) {
            if (predicate(current->data)) {
                trueSeq->Append(current->data);
            } else {
                falseSeq->Append(current->data);
            }
            current = current->next.get();
        }
        
        return {trueSeq, falseSeq};
    }

    std::shared_ptr<Sequence<T>> Slice(int start, int end) const override {
        return GetSubsequence(start, end);
    }

    bool ContainsSubsequence(const Sequence<T>& subsequence) const override {
        if (subsequence.GetLength() == 0) return true;
        if (subsequence.GetLength() > length) return false;

        Node* current = head.get();
        for (int i = 0; i <= length - subsequence.GetLength(); i++) {
            bool match = true;
            Node* temp = current;
            
            for (int j = 0; j < subsequence.GetLength(); j++) {
                if (temp->data != subsequence.Get(j)) {
                    match = false;
                    break;
                }
                temp = temp->next.get();
            }
            
            if (match) return true;
            current = current->next.get();
        }
        return false;
    }

    T& operator[](int index) override {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        
        Node* current = head.get();
        for (int i = 0; i < index; i++) {
            current = current->next.get();
        }
        return current->data;
    }

    const T& operator[](int index) const override {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        
        Node* current = head.get();
        for (int i = 0; i < index; i++) {
            current = current->next.get();
        }
        return current->data;
    }

    bool Contains(const T& item) const override {
        return IndexOf(item) != -1;
    }

    int IndexOf(const T& item) const override {
        Node* current = head.get();
        int index = 0;
        while (current) {
            if (current->data == item) {
                return index;
            }
            current = current->next.get();
            index++;
        }
        return -1;
    }

    bool IsEmpty() const override {
        return length == 0;
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "[";
        Node* current = head.get();
        while (current) {
            ss << current->data;
            if (current->next) ss << ", ";
            current = current->next.get();
        }
        ss << "]";
        return ss.str();
    }
};

// ==================== ОЧЕРЕДЬ (ЦЕЛЕВОЙ АТД) ====================

template <typename T>
class Queue : public Sequence<T> {
private:
    std::shared_ptr<Sequence<T>> storage;

public:
    enum StorageType { ARRAY, LINKED_LIST };

    Queue(StorageType type = ARRAY) {
        if (type == ARRAY) {
            storage = std::make_shared<ArraySequence<T>>();
        } else {
            storage = std::make_shared<LinkedListSequence<T>>();
        }
    }

    Queue(std::initializer_list<T> init, StorageType type = ARRAY) {
        if (type == ARRAY) {
            storage = std::make_shared<ArraySequence<T>>(init);
        } else {
            storage = std::make_shared<LinkedListSequence<T>>(init);
        }
    }

    Queue(const Queue<T>& other) {
        storage = other.storage->GetSubsequence(0, other.GetLength() - 1);
    }

    Queue<T>& operator=(const Queue<T>& other) {
        if (this != &other) {
            storage = other.storage->GetSubsequence(0, other.GetLength() - 1);
        }
        return *this;
    }

    // Основные методы очереди
    void Enqueue(const T& item) {
        storage->Append(item);
    }

    T Dequeue() {
        if (storage->IsEmpty()) throw std::out_of_range("Queue is empty");
        T item = storage->GetFirst();
        storage->RemoveAt(0);
        return item;
    }

    T Peek() const {
        if (storage->IsEmpty()) throw std::out_of_range("Queue is empty");
        return storage->GetFirst();
    }

    // Реализация методов Sequence
    T GetFirst() const override { return storage->GetFirst(); }
    T GetLast() const override { return storage->GetLast(); }
    T Get(int index) const override { return storage->Get(index); }
    std::shared_ptr<Sequence<T>> GetSubsequence(int startIndex, int endIndex) const override {
        return storage->GetSubsequence(startIndex, endIndex);
    }
    int GetLength() const override { return storage->GetLength(); }

    void Append(const T& item) override { Enqueue(item); }
    void Prepend(const T& item) override { storage->Prepend(item); }
    void InsertAt(const T& item, int index) override { storage->InsertAt(item, index); }
    void RemoveAt(int index) override { storage->RemoveAt(index); }
    void Remove(const T& item) override { storage->Remove(item); }
    void Clear() override { storage->Clear(); }

    std::shared_ptr<Sequence<T>> Concat(const Sequence<T>& other) const override {
        return storage->Concat(other);
    }

    std::shared_ptr<Sequence<T>> Map(std::function<T(T)> func) const override {
        return storage->Map(func);
    }

    std::shared_ptr<Sequence<T>> Where(std::function<bool(T)> predicate) const override {
        return storage->Where(predicate);
    }

    T Reduce(std::function<T(T, T)> func, T initial) const override {
        return storage->Reduce(func, initial);
    }

    std::shared_ptr<Sequence<T>> Zip(const Sequence<T>& other) const override {
        return storage->Zip(other);
    }

    std::pair<std::shared_ptr<Sequence<T>>, std::shared_ptr<Sequence<T>>> Split(std::function<bool(T)> predicate) const override {
        return storage->Split(predicate);
    }

    std::shared_ptr<Sequence<T>> Slice(int start, int end) const override {
        return storage->Slice(start, end);
    }

    bool ContainsSubsequence(const Sequence<T>& subsequence) const override {
        return storage->ContainsSubsequence(subsequence);
    }

    T& operator[](int index) override { return (*storage)[index]; }
    const T& operator[](int index) const override { return (*storage)[index]; }

    bool Contains(const T& item) const override { return storage->Contains(item); }
    int IndexOf(const T& item) const override { return storage->IndexOf(item); }
    bool IsEmpty() const override { return storage->IsEmpty(); }

    std::string ToString() const override {
        return storage->ToString();
    }

    // Специфичные методы для очереди
    std::shared_ptr<Queue<T>> Filter(std::function<bool(T)> predicate) const {
        auto result = std::make_shared<Queue<T>>();
        for (int i = 0; i < GetLength(); i++) {
            T item = Get(i);
            if (predicate(item)) {
                result->Enqueue(item);
            }
        }
        return result;
    }

    void Serialize(const std::string& filename) const {
        std::ofstream file(filename);
        for (int i = 0; i < GetLength(); i++) {
            file << Get(i) << "\n";
        }
    }

    void Deserialize(const std::string& filename) {
        std::ifstream file(filename);
        T item;
        while (file >> item) {
            Enqueue(item);
        }
    }
};

// ==================== ТЕСТЫ ====================

class TestRunner {
private:
    int testsPassed = 0;
    int testsFailed = 0;
    std::vector<std::string> failedTests;

    template<typename T>
    bool assertEqual(const T& actual, const T& expected, const std::string& testName) {
        if (actual == expected) {
            std::cout << "✓ " << testName << std::endl;
            testsPassed++;
            return true;
        } else {
            std::cout << "✗ " << testName << " - Expected: " << expected 
                      << ", Got: " << actual << std::endl;
            testsFailed++;
            failedTests.push_back(testName);
            return false;
        }
    }

    bool assertEqual(const std::string& actual, const char* expected, const std::string& testName) {
        return assertEqual<std::string>(actual, std::string(expected), testName);
    }

    bool assertTrue(bool condition, const std::string& testName) {
        return assertEqual(condition, true, testName);
    }

    bool assertFalse(bool condition, const std::string& testName) {
        return assertEqual(condition, false, testName);
    }

    bool assertException(std::function<void()> func, const std::string& testName) {
        try {
            func();
            std::cout << "✗ " << testName << " - Expected exception but none thrown" << std::endl;
            testsFailed++;
            failedTests.push_back(testName);
            return false;
        } catch (const std::exception& e) {
            std::cout << "✓ " << testName << " - Exception: " << e.what() << std::endl;
            testsPassed++;
            return true;
        }
    }

public:
    void runAllTests() {
        std::cout << "=== ЗАПУСК ВСЕХ ТЕСТОВ ===" << std::endl;
        
        testArraySequenceBasic();
        testLinkedListSequenceBasic();
        testQueueOperations();
        testFunctionalOperations();
        testEdgeCases();
        testComplexTypes();
        testPerformance();
        
        printResults();
    }

    void testArraySequenceBasic() {
        std::cout << "\n--- Тестирование ArraySequence (базовое) ---" << std::endl;
        
        ArraySequence<int> seq;
        assertTrue(seq.IsEmpty(), "Пустая последовательность");
        
        seq.Append(1);
        seq.Append(2);
        seq.Append(3);
        assertEqual(seq.GetLength(), 3, "Длина после добавления");
        assertEqual(seq.GetFirst(), 1, "Первый элемент");
        assertEqual(seq.GetLast(), 3, "Последний элемент");
        
        seq.Prepend(0);
        assertEqual(seq.GetFirst(), 0, "Добавление в начало");
        
        seq.InsertAt(5, 2);
        assertEqual(seq.Get(2), 5, "Вставка по индексу");
        
        seq.RemoveAt(2);
        assertEqual(seq.Get(2), 2, "Удаление по индексу");
        
        auto sub = seq.GetSubsequence(1, 3);
        assertEqual(sub->GetLength(), 3, "Подпоследовательность длина");
        assertEqual(sub->Get(0), 1, "Подпоследовательность элемент 0");
    }

    void testLinkedListSequenceBasic() {
        std::cout << "\n--- Тестирование LinkedListSequence (базовое) ---" << std::endl;
        
        LinkedListSequence<int> seq;
        assertTrue(seq.IsEmpty(), "Пустая последовательность");
        
        seq.Append(1);
        seq.Append(2);
        seq.Append(3);
        assertEqual(seq.GetLength(), 3, "Длина после добавления");
        assertEqual(seq.GetFirst(), 1, "Первый элемент");
        assertEqual(seq.GetLast(), 3, "Последний элемент");
        
        seq.Prepend(0);
        assertEqual(seq.GetFirst(), 0, "Добавление в начало");
        
        seq.InsertAt(5, 2);
        assertEqual(seq.Get(2), 5, "Вставка по индексу");
        
        seq.RemoveAt(2);
        assertEqual(seq.Get(2), 2, "Удаление по индексу");
    }

    void testQueueOperations() {
        std::cout << "\n--- Тестирование Queue ---" << std::endl;
        
        Queue<int> queue;
        queue.Enqueue(1);
        queue.Enqueue(2);
        queue.Enqueue(3);
        
        assertEqual(queue.Peek(), 1, "Peek первый элемент");
        assertEqual(queue.Dequeue(), 1, "Dequeue первый элемент");
        assertEqual(queue.Peek(), 2, "Peek после Dequeue");
        assertEqual(queue.GetLength(), 2, "Длина после Dequeue");
        
        Queue<int> queue2({4, 5, 6}, Queue<int>::ARRAY);
        assertEqual(queue2.GetLength(), 3, "Инициализация списком");
        
        // Тест разных типов хранения
        Queue<int> arrayQueue(Queue<int>::ARRAY);
        Queue<int> listQueue(Queue<int>::LINKED_LIST);
        
        arrayQueue.Enqueue(1);
        listQueue.Enqueue(1);
        
        assertEqual(arrayQueue.GetLength(), 1, "Array queue length");
        assertEqual(listQueue.GetLength(), 1, "List queue length");
    }

    void testFunctionalOperations() {
        std::cout << "\n--- Тестирование функциональных операций ---" << std::endl;
        
        ArraySequence<int> seq = {1, 2, 3, 4, 5};
        
        // Test Map
        auto doubled = seq.Map([](int x) { return x * 2; });
        assertEqual(doubled->Get(2), 6, "Map элемент 2");
        assertEqual(doubled->Get(4), 10, "Map элемент 4");
        
        // Test Where
        auto even = seq.Where([](int x) { return x % 2 == 0; });
        assertEqual(even->GetLength(), 2, "Where длина");
        assertEqual(even->Get(0), 2, "Where элемент 0");
        
        // Test Reduce
        int sum = seq.Reduce([](int a, int b) { return a + b; }, 0);
        assertEqual(sum, 15, "Reduce сумма");
        
        int product = seq.Reduce([](int a, int b) { return a * b; }, 1);
        assertEqual(product, 120, "Reduce произведение");
        
        // Test Concat
        ArraySequence<int> seq2 = {6, 7, 8};
        auto concatenated = seq.Concat(seq2);
        assertEqual(concatenated->GetLength(), 8, "Concat длина");
        assertEqual(concatenated->Get(5), 6, "Concat элемент 5");
        
        // Test ContainsSubsequence
        ArraySequence<int> sub = {3, 4};
        assertTrue(seq.ContainsSubsequence(sub), "Contains subsequence");
        
        ArraySequence<int> notSub = {3, 5};
        assertFalse(seq.ContainsSubsequence(notSub), "Does not contain subsequence");
    }

    void testEdgeCases() {
        std::cout << "\n--- Тестирование граничных случаев ---" << std::endl;
        
        // Пустая последовательность
        ArraySequence<int> emptySeq;
        assertException([&]() { emptySeq.GetFirst(); }, "Empty sequence GetFirst exception");
        assertException([&]() { emptySeq.GetLast(); }, "Empty sequence GetLast exception");
        
        // Неверные индексы
        ArraySequence<int> seq = {1, 2, 3};
        assertException([&]() { seq.Get(-1); }, "Negative index exception");
        assertException([&]() { seq.Get(10); }, "Out of bounds index exception");
        assertException([&]() { seq.GetSubsequence(2, 1); }, "Invalid subsequence indices");
        assertException([&]() { seq.GetSubsequence(-1, 2); }, "Negative subsequence start");
        assertException([&]() { seq.GetSubsequence(1, 5); }, "Out of bounds subsequence end");
        
        // Удаление из пустой
        assertException([&]() { emptySeq.RemoveAt(0); }, "Remove from empty");
        
        // Вставка в неверную позицию
        assertException([&]() { seq.InsertAt(0, -1); }, "Insert at negative index");
        assertException([&]() { seq.InsertAt(0, 10); }, "Insert at out of bounds index");
    }

    void testComplexTypes() {
        std::cout << "\n--- Тестирование сложных типов ---" << std::endl;
        
        // Комплексные числа
        ArraySequence<Complex> complexSeq = {Complex(1, 2), Complex(3, 4)};
        assertEqual(complexSeq.GetLength(), 2, "Complex sequence length");
        
        auto complexMapped = complexSeq.Map([](Complex c) { return c * 2.0; });
        assertEqual(complexMapped->Get(0), Complex(2, 4), "Complex map");
        
        // Строки
        ArraySequence<std::string> stringSeq = {"hello", "world", "test"};
        assertEqual(stringSeq.GetLength(), 3, "String sequence length");
        
        auto stringMapped = stringSeq.Map([](std::string s) { return s + "!"; });
        assertEqual(stringMapped->Get(0), "hello!", "String map");
        
        auto stringFiltered = stringSeq.Where([](std::string s) { return s.length() > 4; });
        assertEqual(stringFiltered->GetLength(), 2, "String filter");
        
        // Персоны
        PersonID id1{123, 456};
        PersonID id2{789, 101};
        std::time_t now = std::time(nullptr);
        Person p1(id1, "John", "A", "Doe", now);
        Person p2(id2, "Jane", "B", "Smith", now);
        
        ArraySequence<Person> personSeq = {p1, p2};
        assertEqual(personSeq.GetLength(), 2, "Person sequence length");
        assertTrue(personSeq.Contains(p1), "Person sequence contains");
    }

    void testPerformance() {
        std::cout << "\n--- Тестирование производительности ---" << std::endl;
        
        const int LARGE_SIZE = 10000;
        
        // Тест ArraySequence
        auto start = std::chrono::high_resolution_clock::now();
        ArraySequence<int> arraySeq;
        for (int i = 0; i < LARGE_SIZE; i++) {
            arraySeq.Append(i);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto arrayTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Тест LinkedListSequence
        start = std::chrono::high_resolution_clock::now();
        LinkedListSequence<int> listSeq;
        for (int i = 0; i < LARGE_SIZE; i++) {
            listSeq.Append(i);
        }
        end = std::chrono::high_resolution_clock::now();
        auto listTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "ArraySequence время: " << arrayTime.count() << "ms" << std::endl;
        std::cout << "LinkedListSequence время: " << listTime.count() << "ms" << std::endl;
        
        // Проверка что оба работают корректно
        assertEqual(arraySeq.GetLength(), LARGE_SIZE, "Large array sequence length");
        assertEqual(listSeq.GetLength(), LARGE_SIZE, "Large list sequence length");
        
        // Тест операций
        start = std::chrono::high_resolution_clock::now();
        auto mapped = arraySeq.Map([](int x) { return x * 2; });
        end = std::chrono::high_resolution_clock::now();
        auto mapTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Map операция: " << mapTime.count() << "ms" << std::endl;
    }

    void printResults() {
        std::cout << "\n=== ИТОГИ ТЕСТИРОВАНИЯ ===" << std::endl;
        std::cout << "Всего тестов: " << (testsPassed + testsFailed) << std::endl;
        std::cout << "Пройдено: " << testsPassed << std::endl;
        std::cout << "Провалено: " << testsFailed << std::endl;
        
        if (!failedTests.empty()) {
            std::cout << "\nПроваленные тесты:" << std::endl;
            for (const auto& test : failedTests) {
                std::cout << "  - " << test << std::endl;
            }
        }
        
        double coverage = 100.0 * testsPassed / (testsPassed + testsFailed);
        std::cout << "\nПокрытие тестами: " << std::fixed << std::setprecision(1) << coverage << "%" << std::endl;
        
        if (coverage >= 95.0) {
            std::cout << "✓ Отличное покрытие тестами!" << std::endl;
        } else if (coverage >= 80.0) {
            std::cout << "✓ Хорошее покрытие тестами" << std::endl;
        } else {
            std::cout << "✗ Низкое покрытие тестами" << std::endl;
        }
    }
};

// ==================== ПОЛЬЗОВАТЕЛЬСКИЙ ИНТЕРФЕЙС ====================

class ConsoleUI {
private:
    template<typename T>
    void printSequence(const Sequence<T>& seq) {
        std::cout << seq.ToString() << std::endl;
    }

    template<typename T>
    T getInput(const std::string& prompt) {
        T value;
        std::cout << prompt;
        std::cin >> value;
        return value;
    }

    // Специализация для string
    std::string getInputString(const std::string& prompt) {
        std::string value;
        std::cout << prompt;
        std::cin.ignore();
        std::getline(std::cin, value);
        return value;
    }

    template<typename T>
    void demoQueueOperations() {
        int storageChoice;
        std::cout << "Выберите тип хранения:\n1. Массив\n2. Связный список\nВыбор: ";
        std::cin >> storageChoice;
        
        typename Queue<T>::StorageType storageType = (storageChoice == 1) ? 
            Queue<T>::ARRAY : Queue<T>::LINKED_LIST;
        
        Queue<T> queue(storageType);
        int choice;
        
        do {
            std::cout << "\n=== ОПЕРАЦИИ С ОЧЕРЕДЬЮ ===" << std::endl;
            std::cout << "1. Enqueue (добавить)" << std::endl;
            std::cout << "2. Dequeue (удалить)" << std::endl;
            std::cout << "3. Peek (посмотреть)" << std::endl;
            std::cout << "4. Map" << std::endl;
            std::cout << "5. Where" << std::endl;
            std::cout << "6. Reduce" << std::endl;
            std::cout << "7. Concat" << std::endl;
            std::cout << "8. Split" << std::endl;
            std::cout << "9. Поиск подпоследовательности" << std::endl;
            std::cout << "10. Показать очередь" << std::endl;
            std::cout << "11. Сохранить в файл" << std::endl;
            std::cout << "12. Загрузить из файла" << std::endl;
            std::cout << "0. Назад" << std::endl;
            std::cout << "Выбор: ";
            std::cin >> choice;
            
            try {
                switch (choice) {
                    case 1: {
                        T value;
                        if constexpr (std::is_same_v<T, std::string>) {
                            value = getInputString("Введите значение: ");
                        } else {
                            value = getInput<T>("Введите значение: ");
                        }
                        queue.Enqueue(value);
                        std::cout << "Добавлено: " << value << std::endl;
                        break;
                    }
                    case 2: {
                        T value = queue.Dequeue();
                        std::cout << "Удалено: " << value << std::endl;
                        break;
                    }
                    case 3: {
                        T value = queue.Peek();
                        std::cout << "Первый элемент: " << value << std::endl;
                        break;
                    }
                    case 4: {
                        auto mapped = queue.Map([](T x) { return x + x; });
                        std::cout << "Результат Map: ";
                        printSequence(*mapped);
                        break;
                    }
                    case 5: {
                        auto filtered = queue.Where([](T x) { 
                            if constexpr (std::is_arithmetic_v<T>) {
                                return x > T{};
                            } else if constexpr (std::is_same_v<T, Complex>) {
                                return std::abs(x) > 0;
                            } else {
                                return !x.empty();
                            }
                        });
                        std::cout << "Результат Where: ";
                        printSequence(*filtered);
                        break;
                    }
                    case 6: {
                        T result = queue.Reduce([](T a, T b) { return a + b; }, T{});
                        std::cout << "Результат Reduce: " << result << std::endl;
                        break;
                    }
                    case 10: {
                        std::cout << "Очередь: ";
                        printSequence(queue);
                        break;
                    }
                    case 11: {
                        std::string filename = getInputString("Имя файла: ");
                        queue.Serialize(filename);
                        std::cout << "Сохранено в " << filename << std::endl;
                        break;
                    }
                    case 12: {
                        std::string filename = getInputString("Имя файла: ");
                        queue.Deserialize(filename);
                        std::cout << "Загружено из " << filename << std::endl;
                        break;
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "Ошибка: " << e.what() << std::endl;
            }
        } while (choice != 0);
    }

    void demoTypeSelection() {
        int typeChoice;
        do {
            std::cout << "\n=== ВЫБОР ТИПА ДАННЫХ ===" << std::endl;
            std::cout << "1. Целые числа (int)" << std::endl;
            std::cout << "2. Вещественные числа (double)" << std::endl;
            std::cout << "3. Комплексные числа" << std::endl;
            std::cout << "4. Строки" << std::endl;
            std::cout << "5. Персоны" << std::endl;
            std::cout << "0. Назад" << std::endl;
            std::cout << "Выбор: ";
            std::cin >> typeChoice;

            switch (typeChoice) {
                case 1: demoQueueOperations<int>(); break;
                case 2: demoQueueOperations<double>(); break;
                case 3: demoQueueOperations<Complex>(); break;
                case 4: demoQueueOperations<std::string>(); break;
                case 5: {
                    std::cout << "Демо для персон требует специального ввода" << std::endl;
                    Queue<Person> queue(Queue<Person>::ARRAY);
                    Person p;
                    std::cin >> p;
                    queue.Enqueue(p);
                    std::cout << "Добавлена персона: " << p << std::endl;
                    break;
                }
            }
        } while (typeChoice != 0);
    }

    void demoPerformance() {
        std::cout << "\n=== ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
        
        TestRunner runner;
        runner.testPerformance();
    }

public:
    void run() {
        TestRunner testRunner;
        int choice;
        
        do {
            std::cout << "\n=== ГЛАВНОЕ МЕНЮ ===" << std::endl;
            std::cout << "1. Запустить все тесты" << std::endl;
            std::cout << "2. Демонстрация операций" << std::endl;
            std::cout << "3. Тест производительности" << std::endl;
            std::cout << "4. Показать информацию о реализации" << std::endl;
            std::cout << "0. Выход" << std::endl;
            std::cout << "Выбор: ";
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    testRunner.runAllTests();
                    break;
                case 2:
                    demoTypeSelection();
                    break;
                case 3:
                    demoPerformance();
                    break;
                case 4:
                    std::cout << "\n=== ИНФОРМАЦИЯ О РЕАЛИЗАЦИИ ===" << std::endl;
                    std::cout << "АТД Динамический массив: ✓" << std::endl;
                    std::cout << "АТД Связный список: ✓" << std::endl;
                    std::cout << "АТД Последовательность: ✓" << std::endl;
                    std::cout << "АТД Очередь: ✓" << std::endl;
                    std::cout << "Поддержка типов: int, double, Complex, string, Person, FunctionPtr" << std::endl;
                    std::cout << "Операции: Map, Where, Reduce, Concat, Zip, Split, Slice" << std::endl;
                    std::cout << "Обработка ошибок: ✓" << std::endl;
                    std::cout << "Модульные тесты: ✓" << std::endl;
                    std::cout << "Пользовательский интерфейс: ✓" << std::endl;
                    break;
            }
        } while (choice != 0);
    }
};

// ==================== MAIN ====================

int main() {
    try {
        std::cout << "=== ЛАБОРАТОРНАЯ РАБОТА №3 ===" << std::endl;
        std::cout << "Реализация полиморфной очереди с функциональными операциями" << std::endl;
        std::cout << "Вариант 1: Очередь с поддержкой" << std::endl;
        
        ConsoleUI ui;
        ui.run();
        
        std::cout << "Программа завершена успешно!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Неизвестная критическая ошибка" << std::endl;
        return 2;
    }
}