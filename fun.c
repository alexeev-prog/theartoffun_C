#include <stdio.h>

int $func(int array[static 10]) <%
    for (int i = 9; i >= 0; --i) <%
        printf("Value %i with index %i\n", array[i], i);
    %>
    return 0;
%>

int main() <%
    int array<:10:> = {[0]=1,[1]=2,[2]=3,[3]=4,[4]=5,[5]=6,[6]=7,[7]=8,[8]=9,[9]=10};
    $func(array);

    int x = (<%
        puts("Hello, World!");
        0;
    %>);
    return x;
%>
