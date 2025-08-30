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

    DynamicArray<T>* GetSubArray(int startIndex, int endIndex) const {
        if (startIndex < 0 || endIndex >= size || startIndex > endIndex) 
            throw IndexOutOfRange();
        
        int subSize = endIndex - startIndex + 1;
        DynamicArray<T>* subArray = new DynamicArray<T>(subSize);
        for (int i = 0; i < subSize; i++) {
            subArray->items[i] = items[startIndex + i];
        }
        subArray->size = subSize;
        return subArray;
    }

    DynamicArray<T>* Concat(const DynamicArray<T>* other) const {
        DynamicArray<T>* result = new DynamicArray<T>(size + other->size);
        for (int i = 0; i < size; i++) {
            result->Append(items[i]);
        }
        for (int i = 0; i < other->size; i++) {
            result->Append(other->items[i]);
        }
        return result;
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
        LinkedListNode<T>* start;

    public:
        Iterator(LinkedListNode<T>* startNode) : current(startNode), start(startNode) {}
        
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
            current = start;
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

// ==================== BASE ARRAY SEQUENCE ====================
template <class T>
class BaseArraySequence : public Sequence<T> {
protected:
    DynamicArray<T> array;

public:
    class Iterator : public IIterator<T> {
    private:
        const BaseArraySequence<T>& sequence;
        int currentIndex;

    public:
        Iterator(const BaseArraySequence<T>& seq) : sequence(seq), currentIndex(0) {}
        
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

    BaseArraySequence() : array() {}
    BaseArraySequence(const DynamicArray<T>& arr) : array(arr) {}
    BaseArraySequence(T* items, int count) : array(items, count) {}
    BaseArraySequence(std::initializer_list<T> initList) : array(initList) {}
    BaseArraySequence(const BaseArraySequence<T>& other) : array(other.array) {}

    std::unique_ptr<IIterator<T>> CreateIterator() const override {
        return std::make_unique<Iterator>(*this);
    }

    T GetFirst() const override {
        if (array.GetSize() == 0) throw EmptySequenceException();
        return array.Get(0);
    }

    T GetLast() const override {
        if (array.GetSize() == 0) throw EmptySequenceException();
        return array.Get(array.GetSize() - 1);
    }

    T Get(int index) const override {
        return array.Get(index);
    }

    int GetLength() const override {
        return array.GetSize();
    }

    BaseArraySequence<T>* Append(T item) const override {
        BaseArraySequence<T>* newSequence = new BaseArraySequence<T>(*this);
        newSequence->array.Append(item);
        return newSequence;
    }

    BaseArraySequence<T>* Prepend(T item) const override {
        BaseArraySequence<T>* newSequence = new BaseArraySequence<T>(*this);
        newSequence->array.Prepend(item);
        return newSequence;
    }

    BaseArraySequence<T>* InsertAt(T item, int index) const override {
        BaseArraySequence<T>* newSequence = new BaseArraySequence<T>(*this);
        newSequence->array.InsertAt(item, index);
        return newSequence;
    }

    BaseArraySequence<T>* GetSubsequence(int startIndex, int endIndex) const override {
        BaseArraySequence<T>* result = new BaseArraySequence<T>();
        for (int i = startIndex; i <= endIndex; i++) {
            result->array.Append(array.Get(i));
        }
        return result;
    }

    BaseArraySequence<T>* Map(std::function<T(T)> func) const override {
        BaseArraySequence<T>* result = new BaseArraySequence<T>();
        for (int i = 0; i < array.GetSize(); i++) {
            result->array.Append(func(array.Get(i)));
        }
        return result;
    }

    BaseArraySequence<T>* Where(std::function<bool(T)> predicate) const override {
        BaseArraySequence<T>* result = new BaseArraySequence<T>();
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

    BaseArraySequence<T>* Zip(const Sequence<T>* other, std::function<T(T, T)> func) const override {
        BaseArraySequence<T>* result = new BaseArraySequence<T>();
        int minLength = std::min(array.GetSize(), other->GetLength());
        for (int i = 0; i < minLength; i++) {
            result->array.Append(func(array.Get(i), other->Get(i)));
        }
        return result;
    }

    BaseArraySequence<T>* Slice(int startIndex, int deleteCount, const Sequence<T>* insertSequence = nullptr) const override {
        BaseArraySequence<T>* result = new BaseArraySequence<T>(*this);
        
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

    void Print() const override {
        array.Print();
    }

    T operator[](int index) const override {
        return array.Get(index);
    }

    Sequence<T>* Clone() const override {
        return new BaseArraySequence<T>(*this);
    }
};

// ==================== MUTABLE ARRAY SEQUENCE ====================
template <class T>
class MutableArraySequence : public BaseArraySequence<T> {
public:
    using BaseArraySequence<T>::BaseArraySequence;

    // Mutable методы - изменяют текущий объект
    void AppendInPlace(T item) {
        this->array.Append(item);
    }

    void PrependInPlace(T item) {
        this->array.Prepend(item);
    }

    void InsertAtInPlace(T item, int index) {
        this->array.InsertAt(item, index);
    }

    T RemoveAtInPlace(int index) {
        return this->array.RemoveAt(index);
    }

    void SetInPlace(int index, T value) {
        this->array.Set(index, value);
    }

    // Immutable версии (наследуются от базового класса)
    MutableArraySequence<T>* Append(T item) const override {
        MutableArraySequence<T>* newSequence = new MutableArraySequence<T>(*this);
        newSequence->AppendInPlace(item);
        return newSequence;
    }

    MutableArraySequence<T>* Prepend(T item) const override {
        MutableArraySequence<T>* newSequence = new MutableArraySequence<T>(*this);
        newSequence->PrependInPlace(item);
        return newSequence;
    }

    MutableArraySequence<T>* InsertAt(T item, int index) const override {
        MutableArraySequence<T>* newSequence = new MutableArraySequence<T>(*this);
        newSequence->InsertAtInPlace(item, index);
        return newSequence;
    }

    Sequence<T>* Clone() const override {
        return new MutableArraySequence<T>(*this);
    }
};

// ==================== IMMUTABLE ARRAY SEQUENCE ====================
template <class T>
class ImmutableArraySequence : public BaseArraySequence<T> {
public:
    using BaseArraySequence<T>::BaseArraySequence;

    // Все операции возвращают новые объекты
    ImmutableArraySequence<T>* Append(T item) const override {
        ImmutableArraySequence<T>* newSequence = new ImmutableArraySequence<T>(*this);
        newSequence->array.Append(item);
        return newSequence;
    }

    ImmutableArraySequence<T>* Prepend(T item) const override {
        ImmutableArraySequence<T>* newSequence = new ImmutableArraySequence<T>(*this);
        newSequence->array.Prepend(item);
        return newSequence;
    }

    ImmutableArraySequence<T>* InsertAt(T item, int index) const override {
        ImmutableArraySequence<T>* newSequence = new ImmutableArraySequence<T>(*this);
        newSequence->array.InsertAt(item, index);
        return newSequence;
    }

    Sequence<T>* Clone() const override {
        return new ImmutableArraySequence<T>(*this);
    }
};

// ==================== ТЕСТЫ ====================
void TestDynamicArray() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ DYNAMIC ARRAY ===" << std::endl;
    
    DynamicArray<int>* arr = new DynamicArray<int>({1, 2, 3, 4, 5});
    std::cout << "Dynamic Array: ";
    arr->Print();
    
    arr->Append(6);
    std::cout << "После Append(6): ";
    arr->Print();
    
    arr->Prepend(0);
    std::cout << "После Prepend(0): ";
    arr->Print();
    
    std::cout << "Элемент по индексу 3: " << arr->Get(3) << std::endl;
    std::cout << "Размер массива: " << arr->GetSize() << std::endl;
    
    // Тест итератора
    std::cout << "Итератор: ";
    auto iterator = arr->CreateIterator();
    while (iterator->HasNext()) {
        std::cout << iterator->Next() << " ";
    }
    std::cout << std::endl;
    
    delete arr;
}

void TestLinkedList() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ LINKED LIST ===" << std::endl;
    
    LinkedList<int>* list = new LinkedList<int>({1, 2, 3, 4, 5});
    std::cout << "Linked List: ";
    list->Print();
    
    list->Append(6);
    std::cout << "После Append(6): ";
    list->Print();
    
    list->Prepend(0);
    std::cout << "После Prepend(0): ";
    list->Print();
    
    LinkedList<int>* subList = list->GetSubList(2, 4);
    std::cout << "Подсписок [2:4]: ";
    subList->Print();
    
    std::cout << "Первый элемент: " << list->GetFirst() << std::endl;
    std::cout << "Последний элемент: " << list->GetLast() << std::endl;
    std::cout << "Элемент по индексу 3: " << list->Get(3) << std::endl;
    
    // Тест итератора
    std::cout << "Итератор: ";
    auto iterator = list->CreateIterator();
    while (iterator->HasNext()) {
        std::cout << iterator->Next() << " ";
    }
    std::cout << std::endl;
    
    delete list;
    delete subList;
}

void TestMutableSequence() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ MUTABLE SEQUENCE ===" << std::endl;
    
    MutableArraySequence<int>* mutableSeq = new MutableArraySequence<int>({1, 2, 3});
    std::cout << "Mutable Sequence: ";
    mutableSeq->Print();
    
    // Изменяем оригинальный объект
    mutableSeq->AppendInPlace(4);
    std::cout << "После AppendInPlace(4): ";
    mutableSeq->Print();
    
    mutableSeq->SetInPlace(1, 99);
    std::cout << "После SetInPlace(1, 99): ";
    mutableSeq->Print();
    
    // Immutable операция возвращает новый объект
    MutableArraySequence<int>* newMutable = mutableSeq->Append(5);
    std::cout << "После Append(5) - новый объект: ";
    newMutable->Print();
    std::cout << "Оригинал неизменен: ";
    mutableSeq->Print();
    
    // Тест Clone
    MutableArraySequence<int>* cloned = dynamic_cast<MutableArraySequence<int>*>(mutableSeq->Clone());
    std::cout << "Клонированная последовательность: ";
    cloned->Print();
    
    delete mutableSeq;
    delete newMutable;
    delete cloned;
}

void TestImmutableSequence() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ IMMUTABLE SEQUENCE ===" << std::endl;
    
    ImmutableArraySequence<int>* immutableSeq = new ImmutableArraySequence<int>({1, 2, 3});
    std::cout << "Immutable Sequence: ";
    immutableSeq->Print();
    
    // Все операции возвращают новые объекты
    ImmutableArraySequence<int>* newImmutable = immutableSeq->Append(4);
    std::cout << "После Append(4) - новый объект: ";
    newImmutable->Print();
    std::cout << "Оригинал неизменен: ";
    immutableSeq->Print();
    
    ImmutableArraySequence<int>* anotherImmutable = newImmutable->Prepend(0);
    std::cout << "После Prepend(0) - ещё один новый объект: ";
    anotherImmutable->Print();
    std::cout << "Предыдущий неизменен: ";
    newImmutable->Print();
    std::cout << "Оригинал всё ещё неизменен: ";
    immutableSeq->Print();
    
    // Тестирование Map-Reduce
    Sequence<int>* mapped = anotherImmutable->Map([](int x) { return x * 2; });
    std::cout << "После Map(x * 2): ";
    mapped->Print();
    
    int sum = anotherImmutable->Reduce([](int a, int b) { return a + b; }, 0);
    std::cout << "Reduce (сумма): " << sum << std::endl;
    
    // Тест Clone
    ImmutableArraySequence<int>* cloned = dynamic_cast<ImmutableArraySequence<int>*>(immutableSeq->Clone());
    std::cout << "Клонированная последовательность: ";
    cloned->Print();
    
    delete immutableSeq;
    delete newImmutable;
    delete anotherImmutable;
    delete mapped;
    delete cloned;
}

void TestTrySemantics() {
    std::cout << "\n=== ТЕСТИРОВАНИЕ TRY-СЕМАНТИКИ ===" << std::endl;
    
    ImmutableArraySequence<int>* seq = new ImmutableArraySequence<int>({1, 2, 3, 4, 5});
    
    Option<int> found = seq->TryFind([](int x) { return x > 3; });
    if (found.IsSome()) {
        std::cout << "Найден элемент > 3: " << found.GetValue() << std::endl;
    } else {
        std::cout << "Элемент > 3 не найден" << std::endl;
    }
    
    Option<int> notFound = seq->TryFind([](int x) { return x > 10; });
    std::cout << "Поиск элемента > 10: " << (notFound.IsNone() ? "Не найден" : "Найден") << std::endl;
    
    Option<int> first = seq->TryGetFirst();
    Option<int> last = seq->TryGetLast();
    Option<int> element = seq->TryGet(2);
    Option<int> invalid = seq->TryGet(10);
    
    if (first.IsSome()) std::cout << "Первый элемент: " << first.GetValue() << std::endl;
    if (last.IsSome()) std::cout << "Последний элемент: " << last.GetValue() << std::endl;
    if (element.IsSome()) std::cout << "Элемент по индексу 2: " << element.GetValue() << std::endl;
    if (invalid.IsNone()) std::cout << "Элемент по индексу 10: Не существует" << std::endl;
    
    delete seq;
}

void DemoMapReduce() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ MAP-REDUCE ===" << std::endl;
    
    ImmutableArraySequence<int>* seq = new ImmutableArraySequence<int>({1, 2, 3, 4, 5});
    std::cout << "Исходная последовательность: ";
    seq->Print();
    
    // Цепочка Map-Reduce операций
    Sequence<int>* processed = seq->Map([](int x) { return x + 10; })
                              ->Where([](int x) { return x % 2 == 0; })
                              ->Map([](int x) { return x * 2; });
    
    std::cout << "После цепочки Map->Where->Map: ";
    processed->Print();
    
    int finalResult = processed->Reduce([](int a, int b) { return a + b; }, 0);
    std::cout << "Финальный результат Reduce: " << finalResult << std::endl;
    
    delete seq;
    delete processed;
}

void DemoSlice() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ SLICE ===" << std::endl;
    
    ImmutableArraySequence<int>* seq = new ImmutableArraySequence<int>({1, 2, 3, 4, 5, 6, 7, 8});
    ImmutableArraySequence<int>* insertSeq = new ImmutableArraySequence<int>({99, 100});
    
    std::cout << "Исходная последовательность: ";
    seq->Print();
    std::cout << "Последовательность для вставки: ";
    insertSeq->Print();
    
    // Slice с вставкой
    Sequence<int>* sliced = seq->Slice(2, 3, insertSeq);
    std::cout << "Slice(2, 3, {99,100}): ";
    sliced->Print();
    
    // Slice с отрицательным индексом
    Sequence<int>* negativeSlice = seq->Slice(-3, 2);
    std::cout << "Slice(-3, 2): ";
    negativeSlice->Print();
    
    delete seq;
    delete insertSeq;
    delete sliced;
    delete negativeSlice;
}

void RunAllTests() {
    std::cout << "=== ЗАПУСК ВСЕХ ТЕСТОВ ===" << std::endl;
    TestDynamicArray();
    TestLinkedList();
    TestMutableSequence();
    TestImmutableSequence();
    TestTrySemantics();
    DemoMapReduce();
    DemoSlice();
}

// ==================== ПОЛЬЗОВАТЕЛЬСКИЙ ИНТЕРФЕЙС ====================
void ShowMainMenu() {
    std::cout << "\n=== ГЛАВНОЕ МЕНЮ ===" << std::endl;
    std::cout << "1. Тестирование Dynamic Array" << std::endl;
    std::cout << "2. Тестирование Linked List" << std::endl;
    std::cout << "3. Тестирование Mutable Sequence" << std::endl;
    std::cout << "4. Тестирование Immutable Sequence" << std::endl;
    std::cout << "5. Тестирование Try-семантики" << std::endl;
    std::cout << "6. Запуск всех тестов" << std::endl;
    std::cout << "7. Демонстрация Map-Reduce" << std::endl;
    std::cout << "8. Демонстрация Slice" << std::endl;
    std::cout << "0. Выход" << std::endl;
    std::cout << "Выберите опцию: ";
}

void RunUI() {
    int choice;
    do {
        ShowMainMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1: TestDynamicArray(); break;
            case 2: TestLinkedList(); break;
            case 3: TestMutableSequence(); break;
            case 4: TestImmutableSequence(); break;
            case 5: TestTrySemantics(); break;
            case 6: RunAllTests(); break;
            case 7: DemoMapReduce(); break;
            case 8: DemoSlice(); break;
            case 0: std::cout << "Выход..." << std::endl; break;
            default: std::cout << "Неверный выбор!" << std::endl;
        }
    } while (choice != 0);
}

// ==================== MAIN ====================
int main() {
    std::cout << "Лабораторная работа №2 - Полная реализация" << std::endl;
    std::cout << "Система для работы с коллекциями данных" << std::endl;
    
    try {
        RunUI();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    
    return 0;
}