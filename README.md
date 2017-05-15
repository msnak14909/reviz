reviz
====

reviz is the regular expression visualize program. (re -> nfa -> dfa -> dfa_opt)

Usage
=====

just type :
```
./reviz 'a(b|c)*'
```
It will produce nfa.dot, dfa.dot, opt.dot

or type :
```
./reviz_dot 'a(b|c)*'
```
It will convert these .dot file into .png file, however, you need to ensure that you have dot program already, if not, you can install 'graphviz' by using pacman, apt ... etc.


Install
========

Dependencies:

`GNU Make`, `GCC`

```shell
$ make
```

and you will see two executable under directory: reviz reviz_dot

Example
========

```
./reviz 'a(b|c)*'
```

DFA:
[[https://github.com/msnak14909/reviz/raw/master/sample/dfa.png]]

Optimized DFA:
[[https://github.com/msnak14909/reviz/raw/master/sample/opt.png]]
