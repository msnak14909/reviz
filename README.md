reviz
====

reviz is the regular expression visualize program. (re -> nfa -> dfa -> dfa_opt)

The html for demo(build by webassembly): <a href="https://rniczh.github.io/reviz/">DEMO</a>


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

Just Enter :
```
./reviz 'a(b|c)*'
```
It will generate nfa.dot, dfa.dot, opt.dot files

Or Enter :
```
./reviz_dot 'a(b|c)*'
```
It will convert these .dot file into .png file, however, you need to ensure that you have `dot` program already, if not, you can install `graphviz` by using pacman, apt ... etc.


Install
========

Dependencies:

`GNU Make`, `GCC`

```shell
$ make
```

and you will see two executable be generated under program root directory: reviz reviz_dot

Example
========

```
./reviz 'a(b|c)*'
```

DFA:<br />
![dfa](https://i.imgur.com/tyst1LK.png)


Optimized DFA:<br />
![opt](https://i.imgur.com/XeLITca.png)
