reviz
====

reviz is the regular expression visualize program. (re -> nfa -> dfa -> dfa_opt)

The html for demo(build by webassembly): <a href="https://cdn.rawgit.com/msnak14909/reviz/0e1451c0/sample/page/index.html">DEMO</a>


Syntax
=====
It's support gramar as shown below:<br />

**cons:**<br />
  AB<br />

**alter:**<br />
  A|B<br />

**kleene**<br />
  A*<br />

**positive:**<br />
  A+<br />

**optional:**<br />
  A?<br />

**bracket:**<br />
  (A)<br />


**and characters:**<br />

_ : epsilon character <br />

<0-9> \<a-z\> \<A-Z\> : alphanumeric character <br />






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

DFA:<br />
![dfa](https://raw.githubusercontent.com/msnak14909/reviz/master/sample/dfa.png)


Optimized DFA:<br />
![opt](https://raw.githubusercontent.com/msnak14909/reviz/master/sample/opt.png)
