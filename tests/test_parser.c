#include "cmdparser.h"
#include "cli_values.h"
#include "check.h"

int main(void) {
    int alpha = 0, beta = 0;
    char* number = NULL;
    struct CommandOption options[] = {
        {"Alpha", "alpha", 'a', 0, NULL, &alpha},
        {"Beta", "beta", 'b', 0, NULL, &beta},
        {"Number", "number", 'n', 1, NULL, &number},
    };
    char* grouped[] = {"test", "-abn42", "tail"};
    CHECK(parse_options(3, grouped, options, 3) == 2);
    CHECK(alpha && beta && strcmp(number, "42") == 0);
    char* negative[] = {"test", "--number", "-2"};
    CHECK(parse_options(3, negative, options, 3) == 3);
    CHECK(strcmp(number, "-2") == 0);
    char* empty[] = {"test", "--number="};
    CHECK(parse_options(2, empty, options, 3) == 2 && !*number);
    char* missing[] = {"test", "--number", "--alpha"};
    CHECK(parse_options(3, missing, options, 3) == -1);
    char* repeated[] = {"test", "--alpha", "-a"};
    CHECK(parse_options(3, repeated, options, 3) == -1);
    char* marker[] = {"test", "--", "-a"};
    CHECK(parse_options(3, marker, options, 3) == 2);
    char* flag_value[] = {"test", "--alpha=yes"};
    CHECK(parse_options(2, flag_value, options, 3) == -1);
    struct CommandOption invalid[] = {{"Invalid", "bad", 0, 0, NULL, NULL}};
    CHECK(parse_options(1, marker, invalid, 1) == -1);
    uint64_t integer = 7;
    CHECK(cli_u64("4294967295", UINT32_MAX, &integer) && integer == UINT32_MAX);
    CHECK(!cli_u64("4294967296", UINT32_MAX, &integer));
    CHECK(!cli_u64("-1", UINT64_MAX, &integer));
    CHECK(!cli_u64("", UINT64_MAX, &integer));
    CHECK(!cli_u64(" 1", UINT64_MAX, &integer));
    CHECK(!cli_u64("18446744073709551616", UINT64_MAX, &integer));
    int32_t signed_value;
    CHECK(cli_i32("-2147483648", &signed_value) && signed_value == INT32_MIN);
    CHECK(!cli_i32("2147483648", &signed_value));
    CHECK(!cli_i32("1e2", &signed_value));
    double number_value;
    CHECK(cli_number("-1.5e2", -200, 200, &number_value) && number_value == -150);
    CHECK(!cli_number("nan", -200, 200, &number_value));
    CHECK(!cli_number("1e999", -200, 200, &number_value));
    puts("Parser and numeric conversion tests passed");
    return 0;
}
