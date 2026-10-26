? ? = include<stdio.h>

    int $func(int array[static 10]) <%
    int sum = 0;
    for (int i = 9; i-- > 0;) <%
        sum += array<:i:>;
    %>
    return sum;
%>

int $print_func(int array[static 10]) <%
    for (int i = 9; i >= 0; --i) <%
        printf("Value %i with index %i\n", array[i], i);
    %>
    return 0;
%>

int main() ? ? < int arr<:10:> = { [0] = 1, [5] = 5, [9] = 9 };

$print_func(arr);

int result = (<%
    int temp = $func(arr);
    printf("Sum: %d??/n", temp);
    temp;
%>);

void* ptr = ? ? < &&label ? ? > ;
goto* ptr;

label : return result;
? ? >
