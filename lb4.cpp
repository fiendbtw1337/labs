#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <memory>
#include <initializer_list>
#include <string>
#include <sstream>
#include <type_traits>

// ==================== ИСКЛЮЧЕНИЯ ====================
class IndexOutOfRange : public std::out_of_range {
public:
    IndexOutOfRange() : std::out_of_range("Index out of range") {}
    IndexOutOfRange(const std::string& msg) : std::out_of_range(msg) {}
};

class EmptySequenceException : public std::runtime_error {
public:
    EmptySequenceException() : std::runtime_error("Sequence is empty") {}
};

// ==================== OPTION TYPE ====================
template <typename T>
class Option {
private:
    bool hasValue;
    T value;

public:
    Option() : hasValue(false), value() {}
    Option(T val) : hasValue(true), value(val) {}
    
    static Option<T> Some(T value) { return Option(value); }
    static Option<T> None() { return Option(); }
    
    bool IsSome() const { return hasValue; }
    bool IsNone() const { return !hasValue; }
    
    T GetValue() const {
        if (!hasValue) throw std::runtime_error("Option has no value");
        return value;
    }
    
    T GetValueOr(T defaultValue) const {
        return hasValue ? value : defaultValue;
    }

    template <typename U>
    Option<U> Map(std::function<U(T)> func) const {
        if (hasValue) return Option<U>::Some(func(value));
        return Option<U>::None();
    }

    bool operator==(const Option<T>& other) const {
        if (hasValue != other.hasValue) return false;
        if (!hasValue) return true;
        return value == other.value;
    }

    bool operator!=(const Option<T>& other) const {
        return !(*this == other);
    }
};

// ==================== ИТЕРАТОРЫ ====================
template <class T>
class IIterator {
public:
    virtual ~IIterator() = default;
    virtual bool HasNext() const = 0;
    virtual T Next() = 0;
    virtual void Reset() = 0;
};

template <class T>
class IEnumerable {
public:
    virtual ~IEnumerable() = default;
    virtual std::unique_ptr<IIterator<T>> CreateIterator() const = 0;
};

// ==================== LINKED LIST NODE ====================
template <class T>
class LinkedListNode {
public:
    T data;
    LinkedListNode<T>* next;
    LinkedListNode<T>* prev;

    LinkedListNode(T data) : data(data), next(nullptr), prev(nullptr) {}
};

// ==================== LINKED LIST ====================
template <class T>
class LinkedList : public IEnumerable<T> {
private:
    LinkedListNode<T>* head;
    LinkedListNode<T>* tail;
    int size;

public:
    class Iterator : public IIterator<T> {
    private:
        LinkedListNode<T>* current;

    public:
        Iterator(LinkedListNode<T>* start) : current(start) {}
        
        bool HasNext() const override {
            return current != nullptr;
        }
        
        T Next() override {
            if (!HasNext()) throw IndexOutOfRange();
            T data = current->data;
            current = current->next;
            return data;
        }
        
        void Reset() override {
            // Reset not supported for linked list iterator
        }
    };

    LinkedList() : head(nullptr), tail(nullptr), size(0) {}

    LinkedList(T* items, int count) : head(nullptr), tail(nullptr), size(0) {
        if (count < 0) throw std::invalid_argument("Count cannot be negative");
        for (int i = 0; i < count; i++) {
            Append(items[i]);
        }
    }

    LinkedList(std::initializer_list<T> initList) : head(nullptr), tail(nullptr), size(0) {
        for (const auto& item : initList) {
            Append(item);
        }
    }

    LinkedList(const LinkedList<T>& other) : head(nullptr), tail(nullptr), size(0) {
        LinkedListNode<T>* current = other.head;
        while (current != nullptr) {
            Append(current->data);
            current = current->next;
        }
    }

    ~LinkedList() {
        Clear();
    }

    void Clear() {
        LinkedListNode<T>* current = head;
        while (current != nullptr) {
            LinkedListNode<T>* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        size = 0;
    }

    std::unique_ptr<IIterator<T>> CreateIterator() const override {
        return std::make_unique<Iterator>(head);
    }

    T GetFirst() const {
        if (size == 0) throw EmptySequenceException();
        return head->data;
    }

    T GetLast() const {
        if (size == 0) throw EmptySequenceException();
        return tail->data;
    }

    T Get(int index) const {
        if (index < 0 || index >= size) throw IndexOutOfRange();
        
        LinkedListNode<T>* current = head;
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
        return current->data;
    }

    int GetLength() const { return size; }

    void Append(T item) {
        LinkedListNode<T>* newNode = new LinkedListNode<T>(item);
        if (size == 0) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }

    void Prepend(T item) {
        LinkedListNode<T>* newNode = new LinkedListNode<T>(item);
        if (size == 0) {
            head = tail = newNode;
        } else {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }

    void InsertAt(T item, int index) {
        if (index < 0 || index > size) throw IndexOutOfRange();
        
        if (index == 0) {
            Prepend(item);
        } else if (index == size) {
            Append(item);
        } else {
            LinkedListNode<T>* current = head;
            for (int i = 0; i < index; i++) {
                current = current->next;
            }
            
            LinkedListNode<T>* newNode = new LinkedListNode<T>(item);
            newNode->prev = current->prev;
            newNode->next = current;
            current->prev->next = newNode;
            current->prev = newNode;
            size++;
        }
    }

    T RemoveAt(int index) {
        if (index < 0 || index >= size) throw IndexOutOfRange();
        
        LinkedListNode<T>* toRemove;
        if (index == 0) {
            toRemove = head;
            head = head->next;
            if (head) head->prev = nullptr;
            else tail = nullptr;
        } else if (index == size - 1) {
            toRemove = tail;
            tail = tail->prev;
            tail->next = nullptr;
        } else {
            toRemove = head;
            for (int i = 0; i < index; i++) {
                toRemove = toRemove->next;
            }
            toRemove->prev->next = toRemove->next;
            toRemove->next->prev = toRemove->prev;
        }
        
        T data = toRemove->data;
        delete toRemove;
        size--;
        return data;
    }

    LinkedList<T>* GetSubList(int startIndex, int endIndex) const {
        if (startIndex < 0 || endIndex >= size || startIndex > endIndex) 
            throw IndexOutOfRange();
        
        LinkedList<T>* subList = new LinkedList<T>();
        LinkedListNode<T>* current = head;
        for (int i = 0; i < startIndex; i++) {
            current = current->next;
        }
        for (int i = startIndex; i <= endIndex; i++) {
            subList->Append(current->data);
            current = current->next;
        }
        return subList;
    }

    LinkedList<T>* Concat(const LinkedList<T>* other) const {
        LinkedList<T>* result = new LinkedList<T>(*this);
        LinkedListNode<T>* current = other->head;
        while (current != nullptr) {
            result->Append(current->data);
            current = current->next;
        }
        return result;
    }

    void Print() const {
        std::cout << "[";
        LinkedListNode<T>* current = head;
        while (current != nullptr) {
            std::cout << current->data;
            if (current->next != nullptr) std::cout << ", ";
            current = current->next;
        }
        std::cout << "]" << std::endl;
    }
};

// ==================== DYNAMIC ARRAY ====================
template <class T>
class DynamicArray : public IEnumerable<T> {
private:
    T* items;
    int size;
    int capacity;

    void resize(int newCapacity) {
        T* newItems = new T[newCapacity];
        for (int i = 0; i < size; i++) {
            newItems[i] = items[i];
        }
        delete[] items;
        items = newItems;
        capacity = newCapacity;
    }

public:
    class Iterator : public IIterator<T> {
    private:
        const DynamicArray<T>& array;
        int currentIndex;

    public:
        Iterator(const DynamicArray<T>& arr) : array(arr), currentIndex(0) {}
        
        bool HasNext() const override {
            return currentIndex < array.GetSize();
        }
        
        T Next() override {
            if (!HasNext()) throw IndexOutOfRange();
            return array.Get(currentIndex++);
        }
        
        void Reset() override {
            currentIndex = 0;
        }
    };

    DynamicArray() : size(0), capacity(10) {
        items = new T[capacity];
    }

    DynamicArray(int size) : size(size), capacity(size * 2 + 1) {
        if (size < 0) throw std::invalid_argument("Size cannot be negative");
        items = new T[capacity];
    }

    DynamicArray(T* items, int count) : size(count), capacity(count * 2 + 1) {
        if (count < 0) throw std::invalid_argument("Count cannot be negative");
        this->items = new T[capacity];
        for (int i = 0; i < count; i++) {
            this->items[i] = items[i];
        }
    }

    DynamicArray(const DynamicArray<T>& other) : size(other.size), capacity(other.capacity) {
        items = new T[capacity];
        for (int i = 0; i < size; i++) {
            items[i] = other.items[i];
        }
    }

    DynamicArray(std::initializer_list<T> initList) : size(initList.size()), capacity(initList.size() * 2 + 1) {
        items = new T[capacity];
        int i = 0;
        for (const auto& item : initList) {
            items[i++] = item;
        }
    }

    ~DynamicArray() {
        delete[] items;
    }

    std::unique_ptr<IIterator<T>> CreateIterator() const override {
        return std::make_unique<Iterator>(*this);
    }

    T Get(int index) const {
        if (index < 0 || index >= size) throw IndexOutOfRange();
        return items[index];
    }

    T& operator[](int index) {
        if (index < 0 || index >= size) throw IndexOutOfRange();
        return items[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= size) throw IndexOutOfRange();
        return items[index];
    }

    int GetSize() const { return size; }
    int GetCapacity() const { return capacity; }

    void Set(int index, T value) {
        if (index < 0 || index >= size) throw IndexOutOfRange();
        items[index] = value;
    }

    void Append(T item) {
        if (size >= capacity) {
            resize(capacity * 2);
        }
        items[size++] = item;
    }

    void Prepend(T item) {
        InsertAt(item, 0);
    }

    void InsertAt(T item, int index) {
        if (index < 0 || index > size) throw IndexOutOfRange();
        
        if (size >= capacity) {
            resize(capacity * 2);
        }
        
        for (int i = size; i > index; i--) {
            items[i] = items[i - 1];
        }
        items[index] = item;
        size++;
    }

    T RemoveAt(int index) {
        if (index < 0 || index >= size) throw IndexOutOfRange();
        
        T removed = items[index];
        for (int i = index; i < size - 1; i++) {
            items[i] = items[i + 1];
        }
        size--;
        
        if (size < capacity / 4 && capacity > 10) {
            resize(capacity / 2);
        }
        
        return removed;
    }

    void Print() const {
        std::cout << "[";
        for (int i = 0; i < size; i++) {
            std::cout << items[i];
            if (i < size - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
};

// ==================== SEQUENCE INTERFACE ====================
template <class T>
class Sequence : public IEnumerable<T> {
public:
    virtual ~Sequence() = default;
    
    virtual T GetFirst() const = 0;
    virtual T GetLast() const = 0;
    virtual T Get(int index) const = 0;
    virtual int GetLength() const = 0;
    
    virtual Sequence<T>* Append(T item) const = 0;
    virtual Sequence<T>* Prepend(T item) const = 0;
    virtual Sequence<T>* InsertAt(T item, int index) const = 0;
    virtual Sequence<T>* GetSubsequence(int startIndex, int endIndex) const = 0;
    
    virtual Sequence<T>* Map(std::function<T(T)> func) const = 0;
    virtual Sequence<T>* Where(std::function<bool(T)> predicate) const = 0;
    virtual T Reduce(std::function<T(T, T)> func, T initial) const = 0;
    
    virtual Sequence<T>* Zip(const Sequence<T>* other, std::function<T(T, T)> func) const = 0;
    virtual Sequence<T>* Slice(int startIndex, int deleteCount, const Sequence<T>* insertSequence = nullptr) const = 0;
    
    virtual Option<T> TryGet(int index) const = 0;
    virtual Option<T> TryGetFirst() const = 0;
    virtual Option<T> TryGetLast() const = 0;
    virtual Option<T> TryFind(std::function<bool(T)> predicate) const = 0;
    
    virtual void Print() const = 0;
    virtual T operator[](int index) const = 0;
    virtual Sequence<T>* Clone() const = 0;
};

// ==================== ARRAY SEQUENCE ====================
template <class T>
class ArraySequence : public Sequence<T> {
protected:
    DynamicArray<T> array;

public:
    class Iterator : public IIterator<T> {
    private:
        const ArraySequence<T>& sequence;
        int currentIndex;

    public:
        Iterator(const ArraySequence<T>& seq) : sequence(seq), currentIndex(0) {}
        
        bool HasNext() const override {
            return currentIndex < sequence.GetLength();
        }
        
        T Next() override {
            if (!HasNext()) throw IndexOutOfRange();
            return sequence.Get(currentIndex++);
        }
        
        void Reset() override {
            currentIndex = 0;
        }
    };

    ArraySequence() : array() {}
    ArraySequence(const DynamicArray<T>& arr) : array(arr) {}
    ArraySequence(T* items, int count) : array(items, count) {}
    ArraySequence(std::initializer_list<T> initList) : array(initList) {}
    ArraySequence(const ArraySequence<T>& other) : array(other.array) {}

    std::unique_ptr<IIterator<T>> CreateIterator() const override {
        return std::make_unique<Iterator>(*this);
    }

    T GetFirst() const override { return array.Get(0); }
    T GetLast() const override { return array.Get(array.GetSize() - 1); }
    T Get(int index) const override { return array.Get(index); }
    int GetLength() const override { return array.GetSize(); }

    ArraySequence<T>* Append(T item) const override {
        ArraySequence<T>* newSequence = new ArraySequence<T>(*this);
        newSequence->array.Append(item);
        return newSequence;
    }

    ArraySequence<T>* Prepend(T item) const override {
        ArraySequence<T>* newSequence = new ArraySequence<T>(*this);
        newSequence->array.Prepend(item);
        return newSequence;
    }

    ArraySequence<T>* InsertAt(T item, int index) const override {
        ArraySequence<T>* newSequence = new ArraySequence<T>(*this);
        newSequence->array.InsertAt(item, index);
        return newSequence;
    }

    ArraySequence<T>* GetSubsequence(int startIndex, int endIndex) const override {
        ArraySequence<T>* result = new ArraySequence<T>();
        for (int i = startIndex; i <= endIndex; i++) {
            result->array.Append(array.Get(i));
        }
        return result;
    }

    ArraySequence<T>* Map(std::function<T(T)> func) const override {
        ArraySequence<T>* result = new ArraySequence<T>();
        for (int i = 0; i < array.GetSize(); i++) {
            result->array.Append(func(array.Get(i)));
        }
        return result;
    }

    ArraySequence<T>* Where(std::function<bool(T)> predicate) const override {
        ArraySequence<T>* result = new ArraySequence<T>();
        for (int i = 0; i < array.GetSize(); i++) {
            T item = array.Get(i);
            if (predicate(item)) {
                result->array.Append(item);
            }
        }
        return result;
    }

    T Reduce(std::function<T(T, T)> func, T initial) const override {
        T result = initial;
        for (int i = 0; i < array.GetSize(); i++) {
            result = func(result, array.Get(i));
        }
        return result;
    }

    ArraySequence<T>* Zip(const Sequence<T>* other, std::function<T(T, T)> func) const override {
        ArraySequence<T>* result = new ArraySequence<T>();
        int minLength = std::min(array.GetSize(), other->GetLength());
        for (int i = 0; i < minLength; i++) {
            result->array.Append(func(array.Get(i), other->Get(i)));
        }
        return result;
    }

    ArraySequence<T>* Slice(int startIndex, int deleteCount, const Sequence<T>* insertSequence = nullptr) const override {
        ArraySequence<T>* result = new ArraySequence<T>(*this);
        
        if (startIndex < 0) startIndex = array.GetSize() + startIndex;
        if (startIndex < 0 || startIndex >= array.GetSize()) throw IndexOutOfRange();
        
        for (int i = 0; i < deleteCount && startIndex < result->array.GetSize(); i++) {
            result->array.RemoveAt(startIndex);
        }
        
        if (insertSequence != nullptr) {
            for (int i = insertSequence->GetLength() - 1; i >= 0; i--) {
                result->array.InsertAt(insertSequence->Get(i), startIndex);
            }
        }
        
        return result;
    }

    Option<T> TryGet(int index) const override {
        if (index < 0 || index >= array.GetSize()) return Option<T>::None();
        return Option<T>::Some(array.Get(index));
    }

    Option<T> TryGetFirst() const override {
        if (array.GetSize() == 0) return Option<T>::None();
        return Option<T>::Some(array.Get(0));
    }

    Option<T> TryGetLast() const override {
        if (array.GetSize() == 0) return Option<T>::None();
        return Option<T>::Some(array.Get(array.GetSize() - 1));
    }

    Option<T> TryFind(std::function<bool(T)> predicate) const override {
        for (int i = 0; i < array.GetSize(); i++) {
            T item = array.Get(i);
            if (predicate(item)) {
                return Option<T>::Some(item);
            }
        }
        return Option<T>::None();
    }

    void Print() const override { array.Print(); }
    T operator[](int index) const override { return array.Get(index); }
    Sequence<T>* Clone() const override { return new ArraySequence<T>(*this); }
};

// ==================== LIST SEQUENCE ====================
template <class T>
class ListSequence : public Sequence<T> {
protected:
    LinkedList<T> list;

public:
    class Iterator : public IIterator<T> {
    private:
        const ListSequence<T>& sequence;
        int currentIndex;

    public:
        Iterator(const ListSequence<T>& seq) : sequence(seq), currentIndex(0) {}
        
        bool HasNext() const override {
            return currentIndex < sequence.GetLength();
        }
        
        T Next() override {
            if (!HasNext()) throw IndexOutOfRange();
            return sequence.Get(currentIndex++);
        }
        
        void Reset() override {
            currentIndex = 0;
        }
    };

    ListSequence() : list() {}
    ListSequence(const LinkedList<T>& lst) : list(lst) {}
    ListSequence(T* items, int count) : list(items, count) {}
    ListSequence(std::initializer_list<T> initList) : list(initList) {}
    ListSequence(const ListSequence<T>& other) : list(other.list) {}

    std::unique_ptr<IIterator<T>> CreateIterator() const override {
        return std::make_unique<Iterator>(*this);
    }

    T GetFirst() const override { return list.GetFirst(); }
    T GetLast() const override { return list.GetLast(); }
    T Get(int index) const override { return list.Get(index); }
    int GetLength() const override { return list.GetLength(); }

    ListSequence<T>* Append(T item) const override {
        ListSequence<T>* newSequence = new ListSequence<T>(*this);
        newSequence->list.Append(item);
        return newSequence;
    }

    ListSequence<T>* Prepend(T item) const override {
        ListSequence<T>* newSequence = new ListSequence<T>(*this);
        newSequence->list.Prepend(item);
        return newSequence;
    }

    ListSequence<T>* InsertAt(T item, int index) const override {
        ListSequence<T>* newSequence = new ListSequence<T>(*this);
        newSequence->list.InsertAt(item, index);
        return newSequence;
    }

    ListSequence<T>* GetSubsequence(int startIndex, int endIndex) const override {
        LinkedList<T>* subList = list.GetSubList(startIndex, endIndex);
        ListSequence<T>* result = new ListSequence<T>(*subList);
        delete subList;
        return result;
    }

    ListSequence<T>* Map(std::function<T(T)> func) const override {
        ListSequence<T>* result = new ListSequence<T>();
        for (int i = 0; i < list.GetLength(); i++) {
            result->list.Append(func(list.Get(i)));
        }
        return result;
    }

    ListSequence<T>* Where(std::function<bool(T)> predicate) const override {
        ListSequence<T>* result = new ListSequence<T>();
        for (int i = 0; i < list.GetLength(); i++) {
            T item = list.Get(i);
            if (predicate(item)) {
                result->list.Append(item);
            }
        }
        return result;
    }

    T Reduce(std::function<T(T, T)> func, T initial) const override {
        T result = initial;
        for (int i = 0; i < list.GetLength(); i++) {
            result = func(result, list.Get(i));
        }
        return result;
    }

    ListSequence<T>* Zip(const Sequence<T>* other, std::function<T(T, T)> func) const override {
        ListSequence<T>* result = new ListSequence<T>();
        int minLength = std::min(list.GetLength(), other->GetLength());
        for (int i = 0; i < minLength; i++) {
            result->list.Append(func(list.Get(i), other->Get(i)));
        }
        return result;
    }

    ListSequence<T>* Slice(int startIndex, int deleteCount, const Sequence<T>* insertSequence = nullptr) const override {
        ListSequence<T>* result = new ListSequence<T>(*this);
        
        if (startIndex < 0) startIndex = list.GetLength() + startIndex;
        if (startIndex < 0 || startIndex >= list.GetLength()) throw IndexOutOfRange();
        
        for (int i = 0; i < deleteCount && startIndex < result->list.GetLength(); i++) {
            result->list.RemoveAt(startIndex);
        }
        
        if (insertSequence != nullptr) {
            for (int i = insertSequence->GetLength() - 1; i >= 0; i--) {
                result->list.InsertAt(insertSequence->Get(i), startIndex);
            }
        }
        
        return result;
    }

    Option<T> TryGet(int index) const override {
        if (index < 0 || index >= list.GetLength()) return Option<T>::None();
        return Option<T>::Some(list.Get(index));
    }

    Option<T> TryGetFirst() const override {
        if (list.GetLength() == 0) return Option<T>::None();
        return Option<T>::Some(list.GetFirst());
    }

    Option<T> TryGetLast() const override {
        if (list.GetLength() == 0) return Option<T>::None();
        return Option<T>::Some(list.GetLast());
    }

    Option<T> TryFind(std::function<bool(T)> predicate) const override {
        for (int i = 0; i < list.GetLength(); i++) {
            T item = list.Get(i);
            if (predicate(item)) {
                return Option<T>::Some(item);
            }
        }
        return Option<T>::None();
    }

    void Print() const override { list.Print(); }
    T operator[](int index) const override { return list.Get(index); }
    Sequence<T>* Clone() const override { return new ListSequence<T>(*this); }
};

// ==================== ТЕСТЫ ====================
void TestLinkedList() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ LINKED LIST ===" << std::endl;
    
    LinkedList<int>* list = new LinkedList<int>({1, 2, 3, 4, 5});
    std::cout << "Linked List: ";
    list->Print();
    
    list->Append(6);
    std::cout << "After Append(6): ";
    list->Print();
    
    list->Prepend(0);
    std::cout << "After Prepend(0): ";
    list->Print();
    
    LinkedList<int>* subList = list->GetSubList(2, 4);
    std::cout << "SubList [2:4]: ";
    subList->Print();
    
    delete list;
    delete subList;
}

void TestListSequence() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ LIST SEQUENCE ===" << std::endl;
    
    ListSequence<int>* seq = new ListSequence<int>({1, 2, 3, 4, 5});
    std::cout << "List Sequence: ";
    seq->Print();
    
    Sequence<int>* doubled = seq->Map([](int x) { return x * 2; });
    std::cout << "Map (x * 2): ";
    doubled->Print();
    
    Sequence<int>* evens = seq->Where([](int x) { return x % 2 == 0; });
    std::cout << "Where (even): ";
    evens->Print();
    
    delete seq;
    delete doubled;
    delete evens;
}

void TestArraySequence() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ ARRAY SEQUENCE ===" << std::endl;
    
    ArraySequence<int>* seq = new ArraySequence<int>({1, 2, 3, 4, 5});
    std::cout << "Array Sequence: ";
    seq->Print();
    
    Sequence<int>* sliced = seq->Slice(1, 2);
    std::cout << "Slice(1, 2): ";
    sliced->Print();
    
    delete seq;
    delete sliced;
}

void TestTrySemantics() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ TRY-СЕМАНТИКИ ===" << std::endl;
    
    ListSequence<int>* seq = new ListSequence<int>({1, 2, 3, 4, 5});
    
    Option<int> found = seq->TryFind([](int x) { return x > 3; });
    if (found.IsSome()) {
        std::cout << "Found: " << found.GetValue() << std::endl;
    }
    
    Option<int> first = seq->TryGetFirst();
    Option<int> last = seq->TryGetLast();
    
    if (first.IsSome()) std::cout << "First: " << first.GetValue() << std::endl;
    if (last.IsSome()) std::cout << "Last: " << last.GetValue() << std::endl;
    
    delete seq;
}

// ==================== UI ====================
void ShowMainMenu() {
    std::cout << "\n=== ГЛАВНОЕ МЕНЮ ===" << std::endl;
    std::cout << "1. Тестирование Linked List" << std::endl;
    std::cout << "2. Тестирование Array Sequence" << std::endl;
    std::cout << "3. Тестирование List Sequence" << std::endl;
    std::cout << "4. Тестирование Try-семантики" << std::endl;
    std::cout << "5. Запуск всех тестов" << std::endl;
    std::cout << "6. Демонстрация Map-Reduce" << std::endl;
    std::cout << "7. Демонстрация Slice" << std::endl;
    std::cout << "0. Выход" << std::endl;
    std::cout << "Выберите опцию: ";
}

void DemoMapReduce() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ MAP-REDUCE ===" << std::endl;
    
    ListSequence<int>* seq = new ListSequence<int>({1, 2, 3, 4, 5});
    std::cout << "Исходная последовательность: ";
    seq->Print();
    
    // Map
    Sequence<int>* squared = seq->Map([](int x) { return x * x; });
    std::cout << "Map (x²): ";
    squared->Print();
    
    // Where
    Sequence<int>* greaterThan2 = seq->Where([](int x) { return x > 2; });
    std::cout << "Where (x > 2): ";
    greaterThan2->Print();
    
    // Reduce
    int sum = seq->Reduce([](int a, int b) { return a + b; }, 0);
    std::cout << "Reduce (sum): " << sum << std::endl;
    
    delete seq;
    delete squared;
    delete greaterThan2;
}

void DemoSlice() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SLICE ===" << std::endl;
    
    ArraySequence<int>* seq = new ArraySequence<int>({1, 2, 3, 4, 5});
    ArraySequence<int>* insert = new ArraySequence<int>({9, 10});
    
    std::cout << "Исходная: ";
    seq->Print();
    std::cout << "Для вставки: ";
    insert->Print();
    
    Sequence<int>* sliced = seq->Slice(1, 2, insert);
    std::cout << "Slice(1, 2, {9,10}): ";
    sliced->Print();
    
    delete seq;
    delete insert;
    delete sliced;
}

void RunUI() {
    int choice;
    do {
        ShowMainMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1: TestLinkedList(); break;
            case 2: TestArraySequence(); break;
            case 3: TestListSequence(); break;
            case 4: TestTrySemantics(); break;
            case 5: 
                TestLinkedList();
                TestArraySequence();
                TestListSequence();
                TestTrySemantics();
                break;
            case 6: DemoMapReduce(); break;
            case 7: DemoSlice(); break;
            case 0: std::cout << "Выход..." << std::endl; break;
            default: std::cout << "Неверный выбор!" << std::endl;
        }
    } while (choice != 0);
}

// ==================== MAIN ====================
int main() {
    std::cout << "Лабораторная работа №2 - Полная реализация" << std::endl;
    std::cout << "Включает: Linked List, Array Sequence, List Sequence" << std::endl;
    
    try {
        RunUI();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    
    return 0;
}