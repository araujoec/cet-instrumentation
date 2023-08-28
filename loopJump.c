int main()
{
    int i = 100000;
    if (i > 0)
    {
        i--;
        asm(
            "mov    $0x0,%rax;"
            "lea    -0x18(%rip),%rax;"
            "jmp *%rax");
    }
    return (0);
}
