int main()
{
    int x = 5;
    int y = 10;
    int v[10];
    int soma = x + y;
    if (soma > 10)
    {
        printf("Soma é maior que 10\n");
    }
    else
    {
        printf("Soma é menor ou igual a 10\n");
    }
    int i = 0;
    while (i < 5)
    {
        printf("Contador: %d\n", i);
        i = i + 1;
    }
    return 0;
}