# Instrumentação de binários legados para compatibilização com Intel CET

Esse é um repositório descritivo de como replicar os experimentos do artigo [Instrumentação de binários legados para compatibilização com Intel CET](artigo/WTICG23_Artigo_235074.pdf).

## Requisitos
- Sistema Operacional *Linux* (ou derivado do *Unix*)
- Compilador *GCC* (versão utilizada: 11)
- [*objdump*](https://www.gnu.org/software/binutils/), [*dd*](https://www.gnu.org/software/coreutils/) e [*GDB*](https://www.sourceware.org/gdb/) (utilitários nativos do *Linux/Unix*)
- [*CFGgrind*](https://github.com/rimsa/CFGgrind)
- [*dot*](https://graphviz.org/)

## Experimento
O experimento pode ser inteiramente reproduzido pelo terminal de linha de comandos com o auxílio de ferramentas de visualização de imagens e de arquivos de texto. O programa [*loopJump.c*](loopJump.c) foi compilado com a versão 11 do *GCC* por meio do seguinte comando no terminal de linha de comando:

    $ gcc -fcf-protection=none -o loopJump loopJump.c

Para visualizar o arquivo objeto do programa *loopJump.c* e executar a análise estática, a fim de identificar as instruções de desvio indireto, deve-se
utilizar o seguinte comando:

    $ objdump -d loopJump


A saída desse comando pode ser comparada ao conteúdo do arquivo
[*loopJump-dump.txt*](loopJump-dump.txt). No endereço *114c* da
saída deste comando deve-se encontrar a instrução de desvio indireto com a
instrução *jump* (*jmp \*%rax*). Nesse caso, o *objdump* já consegue calcular e identificar o endereço de destino armazenado no registrador *rax*, conforme pode ser observado na linha anterior ao endereço mencionado. Entretanto, ainda assim, a análise dinâmica pode ser realizada.

Para gerar o arquivo de mapeamento das instruções *assembly* com o nome
[*loopJump.map*](loopJump.map), utiliza-se o seguinte comando:

    $ cfggrind_asmmap ./loopJump > loopJump.map


Com esse arquivo é possível gerar outros dois arquivo de extensão *.cfg*
e *.dot* que são utilizados para construir a imagem do grafo de fluxo de
controle das função *main* por meio do seguinte comando:

    $ valgrind -q --tool=cfggrind --cfg-outfile=loopJump.cfg --instrs-map=loopJump.map --cfg-dump=main ./loopJump


A partir do arquivo de extensão *.dot* gerado, é possível então
construir a imagem com o comando:

    $ dot -Tpng -o loopJump.png cfg-0x109129.dot

A imagem construída permite a visualização do endereço de destino da instrução
de salto indireto *jump* e, a partir dela, criar os arquivos de remendo
e trampolim. Esses arquivos devem ser construído após a conversão das
instruções *assembly* para seus valores correspondetes, representados em hexadecimal. Para isso, os comandos a seguir criam os arquivos binários de remendo e trampolim, respectivamente:


    $ echo -e -n    "\xf3\x0f\x1e\xfa\xe9\x28\x00\x00\x00\x90" > remendo.bin
    $ echo -e -n "\x83\x7d\xfc\x00\x7e\xe3\x83\x6d\xfc\x01\xe9\xca\xff\xff\xff" > trampolim.bin


O utilitário *dd* permite a sobrescrita do binário loopJump nos
endereços desejados e pode ser feito com os seguintes comandos:

    $ dd if=remendo.bin of=loopJump bs=1 count=10 seek=4404 conv=notrunc
    $ dd if=trampolim.bin of=loopJump bs=1 count=15 seek=4453 conv=notrunc


Por fim, para verificar a execução do programa passando por cada instrução após
a instrumentação, o utilitário *GDB* pode ser utilizado com o seguinte
comando:

    $ gdb loopJump

O comando a seguir exibe a função *main* em instruções *assembly*:

    $ disassemble/r main

Um *breakpoint* pode ser colocado no endereço [*address*] desejado por meio do comando:

    $ break *0x[address]

Para visualizar cada instrução, a seguinte sequência de comandos deve ser executada:

    $ set disassemble-next-line on
    $ show disassemble-next-line
    $ run

E, finalmente, para iterar pelas instruções, utilize o seguinte comando:

    $ stepi
