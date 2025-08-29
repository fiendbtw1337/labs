#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// ==================== ОПРЕДЕЛЕНИЕ ТИПОВ И СТРУКТУР ====================

typedef struct {
    double real;
    double imag;
} ComplexNumber;  

typedef struct {
    size_t elem_size;
    void (*elem_sum)(void* res, void* a, void* b);
    void (*elem_mult)(void* res, void* a, void* b);
    void (*elem_print)(void* elem);
    void* (*elem_input)(void* elem);
    int (*elem_compare)(void* a, void* b);
    void (*elem_zero)(void* elem);
    void (*elem_free)(void* elem);
} FieldInfo;

typedef struct {
    void* data;
    size_t size;
    size_t capacity;
    FieldInfo* type_info;
} Vector;

// ==================== ФУНКЦИИ ДЛЯ КОМПЛЕКСНЫХ ЧИСЕЛ ====================

void complex_sum(void* res, void* a, void* b) {
    ComplexNumber* c1 = (ComplexNumber*)a;
    ComplexNumber* c2 = (ComplexNumber*)b;
    ComplexNumber* result = (ComplexNumber*)res;
    
    result->real = c1->real + c2->real;
    result->imag = c1->imag + c2->imag;
}

void complex_mult(void* res, void* a, void* b) {
    ComplexNumber* c1 = (ComplexNumber*)a;
    ComplexNumber* c2 = (ComplexNumber*)b;
    ComplexNumber* result = (ComplexNumber*)res;
    
    result->real = c1->real * c2->real - c1->imag * c2->imag;
    result->imag = c1->real * c2->imag + c1->imag * c2->real;
}

void complex_print(void* elem) {
    ComplexNumber* c = (ComplexNumber*)elem;
    if (fabs(c->imag) < 1e-9) {
        printf("%.2f", c->real);
    } else if (fabs(c->real) < 1e-9) {
        printf("%.2fi", c->imag);
    } else if (c->imag > 0) {
        printf("%.2f + %.2fi", c->real, c->imag);
    } else {
        printf("%.2f - %.2fi", c->real, fabs(c->imag));
    }
}

void* complex_input(void* elem) {
    if (!elem) elem = malloc(sizeof(ComplexNumber));
    ComplexNumber* c = (ComplexNumber*)elem;
    
    printf("Введите действительную часть: ");
    if (scanf("%lf", &c->real) != 1) {
        printf("Ошибка ввода! Установлено значение 0.0\n");
        c->real = 0.0;
    }
    
    printf("Введите мнимую часть: ");
    if (scanf("%lf", &c->imag) != 1) {
        printf("Ошибка ввода! Установлено значение 0.0\n");
        c->imag = 0.0;
    }
    
    // Очистка буфера ввода
    int ch;  
    while ((ch = getchar()) != '\n' && ch != EOF);
    
    return elem;
}

int complex_compare(void* a, void* b) {
    ComplexNumber* c1 = (ComplexNumber*)a;
    ComplexNumber* c2 = (ComplexNumber*)b;
    
    double diff_real = c1->real - c2->real;
    double diff_imag = c1->imag - c2->imag;
    
    if (fabs(diff_real) < 1e-9 && fabs(diff_imag) < 1e-9) return 0;
    if (fabs(diff_real) > fabs(diff_imag)) {
        return (diff_real > 0) ? 1 : -1;
    } else {
        return (diff_imag > 0) ? 1 : -1;
    }
}

void complex_zero(void* elem) {
    ComplexNumber* c = (ComplexNumber*)elem;
    c->real = 0.0;
    c->imag = 0.0;
}

void complex_free(void* elem) {
    free(elem);
}

FieldInfo* get_complex_field_info() {
    static FieldInfo* complex_field_info = NULL;
    if (!complex_field_info) {
        complex_field_info = malloc(sizeof(FieldInfo));
        complex_field_info->elem_size = sizeof(ComplexNumber);
        complex_field_info->elem_sum = complex_sum;
        complex_field_info->elem_mult = complex_mult;
        complex_field_info->elem_print = complex_print;
        complex_field_info->elem_input = complex_input;
        complex_field_info->elem_compare = complex_compare;
        complex_field_info->elem_zero = complex_zero;
        complex_field_info->elem_free = complex_free;
    }
    return complex_field_info;
}

// ==================== ФУНКЦИИ ДЛЯ ЦЕЛЫХ ЧИСЕЛ ====================

void int_sum(void* res, void* a, void* b) {
    *(int*)res = *(int*)a + *(int*)b;
}

void int_mult(void* res, void* a, void* b) {
    *(int*)res = *(int*)a * *(int*)b;
}

void int_print(void* elem) {
    printf("%d", *(int*)elem);
}

void* int_input(void* elem) {
    if (!elem) elem = malloc(sizeof(int));
    printf("Введите целое число: ");
    if (scanf("%d", (int*)elem) != 1) {
        printf("Ошибка ввода! Установлено значение 0\n");
        *(int*)elem = 0;
    }
    
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    
    return elem;
}

int int_compare(void* a, void* b) {
    return *(int*)a - *(int*)b;
}

void int_zero(void* elem) {
    *(int*)elem = 0;
}

void int_free(void* elem) {
    free(elem);
}

FieldInfo* get_int_field_info() {
    static FieldInfo* int_field_info = NULL;
    if (!int_field_info) {
        int_field_info = malloc(sizeof(FieldInfo));
        int_field_info->elem_size = sizeof(int);
        int_field_info->elem_sum = int_sum;
        int_field_info->elem_mult = int_mult;
        int_field_info->elem_print = int_print;
        int_field_info->elem_input = int_input;
        int_field_info->elem_compare = int_compare;
        int_field_info->elem_zero = int_zero;
        int_field_info->elem_free = int_free;
    }
    return int_field_info;
}

// ==================== ФУНКЦИИ ДЛЯ ВЕЩЕСТВЕННЫХ ЧИСЕЛ ====================

void double_sum(void* res, void* a, void* b) {
    *(double*)res = *(double*)a + *(double*)b;
}

void double_mult(void* res, void* a, void* b) {
    *(double*)res = *(double*)a * *(double*)b;
}

void double_print(void* elem) {
    printf("%.2f", *(double*)elem);
}

void* double_input(void* elem) {
    if (!elem) elem = malloc(sizeof(double));
    printf("Введите вещественное число: ");
    if (scanf("%lf", (double*)elem) != 1) {
        printf("Ошибка ввода! Установлено значение 0.0\n");
        *(double*)elem = 0.0;
    }
    
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    
    return elem;
}

int double_compare(void* a, void* b) {
    double diff = *(double*)a - *(double*)b;
    return (fabs(diff) < 1e-9) ? 0 : (diff > 0 ? 1 : -1);
}

void double_zero(void* elem) {
    *(double*)elem = 0.0;
}

void double_free(void* elem) {
    free(elem);
}

FieldInfo* get_double_field_info() {
    static FieldInfo* double_field_info = NULL;
    if (!double_field_info) {
        double_field_info = malloc(sizeof(FieldInfo));
        double_field_info->elem_size = sizeof(double);
        double_field_info->elem_sum = double_sum;
        double_field_info->elem_mult = double_mult;
        double_field_info->elem_print = double_print;
        double_field_info->elem_input = double_input;
        double_field_info->elem_compare = double_compare;
        double_field_info->elem_zero = double_zero;
        double_field_info->elem_free = double_free;
    }
    return double_field_info;
}

// ==================== ОСНОВНЫЕ ФУНКЦИИ ВЕКТОРА ====================

Vector* vector_create(FieldInfo* type_info, size_t initial_capacity) {
    if (!type_info) return NULL;
    if (initial_capacity == 0) initial_capacity = 1;
    
    Vector* vec = malloc(sizeof(Vector));
    if (!vec) return NULL;
    
    vec->data = malloc(type_info->elem_size * initial_capacity);
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    
    vec->size = 0;
    vec->capacity = initial_capacity;
    vec->type_info = type_info;
    return vec;
}

int vector_resize(Vector* vec, size_t new_capacity) {
    if (!vec || new_capacity < vec->size) return 0;
    
    void* new_data = realloc(vec->data, vec->type_info->elem_size * new_capacity);
    if (!new_data) return 0;
    
    vec->data = new_data;
    vec->capacity = new_capacity;
    return 1;
}

int vector_push_back(Vector* vec, void* value) {
    if (!vec || !value) return 0;
    
    if (vec->size >= vec->capacity) {
        size_t new_capacity = (vec->capacity == 0) ? 1 : vec->capacity * 2;
        if (!vector_resize(vec, new_capacity)) return 0;
    }
    
    memcpy((char*)vec->data + vec->size * vec->type_info->elem_size, 
           value, vec->type_info->elem_size);
    vec->size++;
    return 1;
}

void* vector_get(Vector* vec, size_t index) {
    if (!vec || index >= vec->size) return NULL;
    return (char*)vec->data + index * vec->type_info->elem_size;
}

void vector_print(Vector* vec) {
    if (!vec) {
        printf("NULL vector\n");
        return;
    }
    
    printf("[");
    for (size_t i = 0; i < vec->size; i++) {
        vec->type_info->elem_print(vector_get(vec, i));
        if (i < vec->size - 1) printf(", ");
    }
    printf("]\n");
}

void vector_free(Vector* vec) {
    if (vec) {
        free(vec->data);
        free(vec);
    }
}

// ==================== ОПЕРАЦИИ С ВЕКТОРАМИ ====================

Vector* vector_add(Vector* a, Vector* b) {
    if (!a || !b) {
        printf("Ошибка: один из векторов NULL\n");
        return NULL;
    }
    if (a->size != b->size) {
        printf("Ошибка: размеры векторов не совпадают (%zu != %zu)\n", a->size, b->size);
        return NULL;
    }
    if (a->type_info != b->type_info) {
        printf("Ошибка: типы данных векторов не совпадают\n");
        return NULL;
    }
    
    Vector* result = vector_create(a->type_info, a->size);
    if (!result) {
        printf("Ошибка: не удалось создать результирующий вектор\n");
        return NULL;
    }
    
    void* temp = malloc(a->type_info->elem_size);
    if (!temp) {
        vector_free(result);
        printf("Ошибка: не удалось выделить память\n");
        return NULL;
    }
    
    for (size_t i = 0; i < a->size; i++) {
        a->type_info->elem_sum(temp, vector_get(a, i), vector_get(b, i));
        if (!vector_push_back(result, temp)) {
            free(temp);
            vector_free(result);
            printf("Ошибка: не удалось добавить элемент в результат\n");
            return NULL;
        }
    }
    
    free(temp);
    return result;
}

void* scalar_product(Vector* a, Vector* b) {
    if (!a || !b) {
        printf("Ошибка: один из векторов NULL\n");
        return NULL;
    }
    if (a->size != b->size) {
        printf("Ошибка: размеры векторов не совпадают (%zu != %zu)\n", a->size, b->size);
        return NULL;
    }
    if (a->type_info != b->type_info) {
        printf("Ошибка: типы данных векторов не совпадают\n");
        return NULL;
    }
    
    void* result = malloc(a->type_info->elem_size);
    void* temp_mult = malloc(a->type_info->elem_size);
    void* temp_sum = malloc(a->type_info->elem_size);
    
    if (!result || !temp_mult || !temp_sum) {
        free(result); free(temp_mult); free(temp_sum);
        printf("Ошибка: не удалось выделить память\n");
        return NULL;
    }
    
    a->type_info->elem_zero(result);
    
    for (size_t i = 0; i < a->size; i++) {
        a->type_info->elem_mult(temp_mult, vector_get(a, i), vector_get(b, i));
        a->type_info->elem_sum(temp_sum, result, temp_mult);
        memcpy(result, temp_sum, a->type_info->elem_size);
    }
    
    free(temp_mult);
    free(temp_sum);
    return result;
}

// ==================== МОДУЛЬНЫЕ ТЕСТЫ ====================

void test_int_vector() {
    printf("\n=== ТЕСТ ЦЕЛОЧИСЛЕННЫХ ВЕКТОРОВ ===\n");
    
    FieldInfo* int_info = get_int_field_info();
    Vector* vec1 = vector_create(int_info, 3);
    Vector* vec2 = vector_create(int_info, 3);
    
    int values1[] = {1, 2, 3};
    int values2[] = {4, 5, 6};
    
    for (int i = 0; i < 3; i++) {
        vector_push_back(vec1, &values1[i]);
        vector_push_back(vec2, &values2[i]);
    }
    
    printf("Вектор 1: ");
    vector_print(vec1);
    printf("Вектор 2: ");
    vector_print(vec2);
    
    Vector* sum = vector_add(vec1, vec2);
    assert(sum != NULL);
    printf("Сумма: ");
    vector_print(sum);
    assert(*(int*)vector_get(sum, 0) == 5);
    assert(*(int*)vector_get(sum, 1) == 7);
    assert(*(int*)vector_get(sum, 2) == 9);
    
    int* scalar = (int*)scalar_product(vec1, vec2);
    assert(scalar != NULL);
    printf("Скалярное произведение: %d\n", *scalar);
    assert(*scalar == 32);
    
    vector_free(vec1);
    vector_free(vec2);
    vector_free(sum);
    free(scalar);
    printf("✓ Тест целых чисел пройден\n");
}

void test_double_vector() {
    printf("\n=== ТЕСТ ВЕЩЕСТВЕННЫХ ВЕКТОРОВ ===\n");
    
    FieldInfo* double_info = get_double_field_info();
    Vector* vec1 = vector_create(double_info, 3);
    Vector* vec2 = vector_create(double_info, 3);
    
    double values1[] = {1.5, 2.5, 3.5};
    double values2[] = {0.5, 1.5, 2.5};
    
    for (int i = 0; i < 3; i++) {
        vector_push_back(vec1, &values1[i]);
        vector_push_back(vec2, &values2[i]);
    }
    
    printf("Вектор 1: ");
    vector_print(vec1);
    printf("Вектор 2: ");
    vector_print(vec2);
    
    Vector* sum = vector_add(vec1, vec2);
    assert(sum != NULL);
    printf("Сумма: ");
    vector_print(sum);
    
    // Проверка правильности сложения
    assert(fabs(*(double*)vector_get(sum, 0) - 2.0) < 1e-9);
    assert(fabs(*(double*)vector_get(sum, 1) - 4.0) < 1e-9);
    assert(fabs(*(double*)vector_get(sum, 2) - 6.0) < 1e-9);
    
    double* scalar_ptr = (double*)scalar_product(vec1, vec2);
    assert(scalar_ptr != NULL);
    printf("Скалярное произведение: %.2f\n", *scalar_ptr);
    
    // Правильный расчет: 1.5*0.5 + 2.5*1.5 + 3.5*2.5 = 0.75 + 3.75 + 8.75 = 13.25
    assert(fabs(*scalar_ptr - 13.25) < 1e-9);
    
    vector_free(vec1);
    vector_free(vec2);
    vector_free(sum);
    free(scalar_ptr);
    printf("✓ Тест вещественных чисел пройден\n");
}

void test_complex_vector() {
    printf("\n=== ТЕСТ КОМПЛЕКСНЫХ ВЕКТОРОВ ===\n");
    
    FieldInfo* complex_info = get_complex_field_info();
    Vector* vec1 = vector_create(complex_info, 3);
    Vector* vec2 = vector_create(complex_info, 3);
    
    ComplexNumber values1[] = {{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};
    ComplexNumber values2[] = {{2.0, 1.0}, {4.0, 3.0}, {6.0, 5.0}};
    
    for (int i = 0; i < 3; i++) {
        vector_push_back(vec1, &values1[i]);
        vector_push_back(vec2, &values2[i]);
    }
    
    printf("Вектор 1: ");
    vector_print(vec1);
    printf("Вектор 2: ");
    vector_print(vec2);
    
    Vector* sum = vector_add(vec1, vec2);
    assert(sum != NULL);
    printf("Сумма: ");
    vector_print(sum);
    
    // Проверка правильности сложения
    ComplexNumber* sum1 = (ComplexNumber*)vector_get(sum, 0);
    ComplexNumber* sum2 = (ComplexNumber*)vector_get(sum, 1);
    ComplexNumber* sum3 = (ComplexNumber*)vector_get(sum, 2);
    
    assert(fabs(sum1->real - 3.0) < 1e-9);
    assert(fabs(sum1->imag - 3.0) < 1e-9);
    assert(fabs(sum2->real - 7.0) < 1e-9);
    assert(fabs(sum2->imag - 7.0) < 1e-9);
    assert(fabs(sum3->real - 11.0) < 1e-9);
    assert(fabs(sum3->imag - 11.0) < 1e-9);
    
    ComplexNumber* scalar_ptr = (ComplexNumber*)scalar_product(vec1, vec2);
    assert(scalar_ptr != NULL);
    printf("Скалярное произведение: ");
    complex_print(scalar_ptr);
    printf("\n");
    
    
    assert(fabs(scalar_ptr->real - 0.0) < 1e-9);
    assert(fabs(scalar_ptr->imag - 91.0) < 1e-9);
    
    vector_free(vec1);
    vector_free(vec2);
    vector_free(sum);
    free(scalar_ptr);
    printf("✓ Тест комплексных чисел пройден\n");
}

void test_error_handling() {
    printf("\n=== ТЕСТ ОБРАБОТКИ ОШИБОК ===\n");
    
    FieldInfo* int_info = get_int_field_info();
    FieldInfo* double_info = get_double_field_info();
    
    Vector* vec_int = vector_create(int_info, 2);
    Vector* vec_double = vector_create(double_info, 2);
    Vector* vec_short = vector_create(int_info, 2);
    
    int int_val1 = 1, int_val2 = 2;
    double double_val = 1.5;
    
    vector_push_back(vec_int, &int_val1);
    vector_push_back(vec_int, &int_val2);
    vector_push_back(vec_double, &double_val);
    vector_push_back(vec_short, &int_val1);
    
    // Тест несовместимых типов
    printf("Тест несовместимых типов:\n");
    Vector* bad_sum1 = vector_add(vec_int, vec_double);
    assert(bad_sum1 == NULL);
    
    // Тест разных размеров
    printf("Тест разных размеров:\n");
    Vector* bad_sum2 = vector_add(vec_int, vec_short);
    assert(bad_sum2 == NULL);
    
    // Тест NULL указателей
    printf("Тест NULL указателей:\n");
    Vector* bad_sum3 = vector_add(NULL, vec_int);
    assert(bad_sum3 == NULL);
    
    vector_free(vec_int);
    vector_free(vec_double);
    vector_free(vec_short);
    printf("✓ Тест обработки ошибок пройден\n");
}

void test_edge_cases() {
    printf("\n=== ТЕСТ ГРАНИЧНЫХ СЛУЧАЕВ ===\n");
    
    FieldInfo* int_info = get_int_field_info();
    
    // Тест пустого вектора
    Vector* empty = vector_create(int_info, 0);
    assert(empty != NULL);
    assert(vector_get(empty, 0) == NULL);
    
    // Тест добавления в полный вектор
    Vector* small = vector_create(int_info, 1);
    int val1 = 1, val2 = 2;
    vector_push_back(small, &val1);
    vector_push_back(small, &val2); // Должен произойти resize
    
    assert(small->capacity >= 2);
    assert(small->size == 2);
    
    vector_free(empty);
    vector_free(small);
    printf("✓ Тест граничных случаев пройден\n");
}

void run_all_tests() {
    printf("=== ЗАПУСК ВСЕХ ТЕСТОВ ===\n");
    test_int_vector();
    test_double_vector();
    test_complex_vector();
    test_error_handling();
    test_edge_cases();
    printf("\n=== ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО ===\n");
}

// ==================== ПОЛЬЗОВАТЕЛЬСКИЙ ИНТЕРФЕЙС ====================

void print_menu() {
    printf("\n=== ГЛАВНОЕ МЕНЮ ===\n");
    printf("1. Запустить автоматические тесты\n");
    printf("2. Ручное тестирование\n");
    printf("3. Выход\n");
    printf("Выберите опцию: ");
}

void manual_test() {
    printf("\n=== РУЧНОЕ ТЕСТИРОВАНИЕ ===\n");
    
    int choice;
    printf("Выберите тип данных:\n");
    printf("1. Целые числа\n");
    printf("2. Вещественные числа\n");
    printf("3. Комплексные числа\n");
    printf("Выбор: ");
    
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 3) {
        printf("Неверный выбор!\n");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        return;
    }
    
    FieldInfo* info = NULL;
    switch (choice) {
        case 1: info = get_int_field_info(); break;
        case 2: info = get_double_field_info(); break;
        case 3: info = get_complex_field_info(); break;
    }
    
    int size;
    printf("Введите размер векторов: ");
    if (scanf("%d", &size) != 1 || size <= 0) {
        printf("Неверный размер! Используется размер по умолчанию: 2\n");
        size = 2;
    }
    
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    
    Vector* vec1 = vector_create(info, size);
    Vector* vec2 = vector_create(info, size);
    
    if (!vec1 || !vec2) {
        printf("Ошибка создания векторов!\n");
        return;
    }
    
    printf("\nЗаполнение первого вектора:\n");
    void* temp = malloc(info->elem_size);
    for (int i = 0; i < size; i++) {
        printf("Элемент %d: ", i + 1);
        info->elem_input(temp);
        vector_push_back(vec1, temp);
    }
    
    printf("\nЗаполнение второго вектора:\n");
    for (int i = 0; i < size; i++) {
        printf("Элемент %d: ", i + 1);
        info->elem_input(temp);
        vector_push_back(vec2, temp);
    }
    
    free(temp);
    
    printf("\n=== РЕЗУЛЬТАТЫ ===\n");
    printf("Вектор 1: ");
    vector_print(vec1);
    printf("Вектор 2: ");
    vector_print(vec2);
    
    Vector* sum = vector_add(vec1, vec2);
    if (sum) {
        printf("Сумма: ");
        vector_print(sum);
        vector_free(sum);
    }
    
    void* scalar = scalar_product(vec1, vec2);
    if (scalar) {
        printf("Скалярное произведение: ");
        info->elem_print(scalar);
        printf("\n");
        free(scalar);
    }
    
    vector_free(vec1);
    vector_free(vec2);
}

// ==================== ГЛАВНАЯ ФУНКЦИЯ ====================

int main() {
    printf("===============================================\n");
    printf("    ЛАБОРАТОРНАЯ РАБОТА №1: ПОЛИМОРФНЫЙ ВЕКТОР\n");
    printf("===============================================\n");
    
    int choice;
    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Неверный ввод! Попробуйте снова.\n");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            continue;
        }
        
        switch (choice) {
            case 1:
                run_all_tests();
                break;
            case 2:
                manual_test();
                break;
            case 3:
                printf("Выход из программы...\n");
                break;
            default:
                printf("Неверный выбор! Попробуйте снова.\n");
        }
        
    } while (choice != 3);
    
    return 0;
}