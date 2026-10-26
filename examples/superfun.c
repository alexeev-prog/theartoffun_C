? ? = include<stdio.h>

    int $func(int array[static 10]) <%
    int sum = 0;
    for (int i = 9; i-- > 0;) <%
        sum += array<:i:>;
    %>
    return sum;
%>

int main() ? ? < int arr<:10:> = { [0] = 1, [5] = 5, [9] = 9 };

int result = (<%
    int temp = $func(arr);
    printf("Sum: %d??/n", temp);
    temp;
%>);

void* ptr = ? ? < &&label ? ? > ;
goto* ptr;

label : return result;
? ? >
