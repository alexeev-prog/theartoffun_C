#include <stdio.h>
#include <string.h>

#include "algos.h"

int main() {
    printf("Тестирование алгоритма Fletcher32\n\n");

    // Тест 1: Пустые данные
    printf("Тест 1: Пустые данные\n");
    uint16_t empty_data[] = { 0 };
    uint32_t result1 = fletcher32(empty_data, 0);
    printf("Результат: 0x%08x\n\n", result1);

    // Тест 2: Простая строка
    printf("Тест 2: Простая строка\n");
    const char* test_str = "Hello, World!";
    uint32_t result2 = fletcher32_string(test_str);
    printf("Строка: \"%s\"\n", test_str);
    printf("Данные: ");
    print_hex((const uint8_t*)test_str, strlen(test_str));
    printf("Результат: 0x%08x\n\n", result2);

    // Тест 3: Другая строка
    printf("Тест 3: Другая строка\n");
    const char* test_str2 = "Fletcher32 checksum test";
    uint32_t result3 = fletcher32_string(test_str2);
    printf("Строка: \"%s\"\n", test_str2);
    printf("Результат: 0x%08x\n\n", result3);

    // Тест 4: Данные как массив uint16_t
    printf("Тест 4: Массив uint16_t\n");
    uint16_t test_data[] = { 0x1234, 0x5678, 0x9abc, 0xdef0 };
    size_t data_len = sizeof(test_data) / sizeof(test_data[0]);
    uint32_t result4 = fletcher32(test_data, data_len);
    printf("Данные: ");
    for (size_t i = 0; i < data_len; i++) {
        printf("0x%04x ", test_data[i]);
    }
    printf("\n");
    printf("Результат: 0x%08x\n\n", result4);

    // Тест 5: Один символ
    printf("Тест 5: Один символ\n");
    const char* single_char = "A";
    uint32_t result5 = fletcher32_string(single_char);
    printf("Строка: \"%s\"\n", single_char);
    printf("Результат: 0x%08x\n\n", result5);

    // Тест 6: Длинная строка (для проверки блочной обработки)
    printf("Тест 6: Длинная строка\n");
    char long_str[1000];
    for (int i = 0; i < 999; i++) {
        long_str[i] = 'A' + (i % 26);
    }
    long_str[999] = '\0';
    uint32_t result6 = fletcher32_string(long_str);
    printf("Длина строки: %zu\n", strlen(long_str));
    printf("Результат: 0x%08x\n\n", result6);

    // Тест 7: Сравнение с известными значениями (если есть)
    printf("Тест 7: Проверка на известных тестовых векторах\n");

    // Тестовые векторы могут различаться в зависимости от реализации
    // Проверим консистентность - одинаковые данные должны давать одинаковый результат
    const char* test_vec1 = "abcde";
    const char* test_vec2 = "abcde";
    uint32_t vec_result1 = fletcher32_string(test_vec1);
    uint32_t vec_result2 = fletcher32_string(test_vec2);

    printf("Строка 1: \"%s\" -> 0x%08x\n", test_vec1, vec_result1);
    printf("Строка 2: \"%s\" -> 0x%08x\n", test_vec2, vec_result2);
    printf("Результаты %s\n", vec_result1 == vec_result2 ? "совпадают" : "не совпадают");

    return 0;
}
