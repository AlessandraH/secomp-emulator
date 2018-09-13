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
* Para uma entrada de tamanho `15000`: `79674186428//337567503=236`.
