# [Curso: Emuladores do Zero](https://github.com/vandersonmr/emuladoresdo0)

* II SECOMP (UEM) - 10 a 14 de setembro 2018
* Dias 12 a 14 de setembro de 2018
* [Arquitetura definida](https://github.com/vandersonmr/emuladoresdo0/blob/master/arch0.md)

#### Compilar:
```
$ gcc -O3 main.c -std=gnu99 -w
```
#### Executar:
```
$ ./a.out input.s
```
#### Avaliar com `perf`:
```
$ perf stat ./a.out input.s
```
* Para uma entrada de tamanho `15000`:

  * <s>`79674186428//337567503=236`</s>.
  * `6753231623//337567503=20`.

## Com suposto JIT

* Utilizando como entrada `2000000`:
  * Sem JIT 389.152.770 / 6000003 = 64 -- Parece que o overhead para verificar se está compila não está ajudando
  * Com JIT 61.464.500 / 6000003 = 10 -- O código LLVM tá muuito otimizado, é até legal de ver o assembly
