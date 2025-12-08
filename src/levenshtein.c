#include <stdlib.h>
#include <string.h>

int min2(int a, int b) {
    return a < b ? a : b;
}

int min3(int a, int b, int c) {
    return min2(a, min2(b, c));
}

int levenshtein(const char* s1, const char* s2) {
    int n = strlen(s1);
    int m = strlen(s2);

    if (n == 0) {
        return m;
    }
    if (m == 0) {
        return n;
    }

    if (n > m) {
        const char* tmp = s1;
        s1 = s2;
        s2 = tmp;
        int t = n;
        n = m;
        m = t;
    }

    int* prev = (int*)malloc((n + 1) * sizeof(int));
    int* curr = (int*)malloc((n + 1) * sizeof(int));

    for (int i = 0; i <= n; i++) {
        prev[i] = i;
    }

    for (int j = 1; j <= m; j++) {
        curr[0] = j;

        for (int i = 1; i <= n; i++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            curr[i] = min3(prev[i] + 1, curr[i - 1] + 1, prev[i - 1] + cost);
        }

        int* tmp = prev;
        prev = curr;
        curr = tmp;
    }

    int result = prev[n];

    free(prev);
    free(curr);

    return result;
}
