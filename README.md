#### HP10L analyzer
  * mHP10L_anal
<!---
    * maketest is a sample of makefile
    * You can apply MyClass for several main functions.
    * reffered following websites
      * general grammer: [http://shin.hateblo.jp/entry/2012/05/26/231036](http://shin.hateblo.jp/entry/2012/05/26/231036)
      * example1: [http://urin.github.io/posts/2013/simple-makefile-for-clang](http://urin.github.io/posts/2013/simple-makefile-for-clang)
      * example2: [http://boysenberrypi.hatenadiary.jp/entry/2014/03/15/113703](http://boysenberrypi.hatenadiary.jp/entry/2014/03/15/113703)
      * makefile function: [https://qiita.com/chibi929/items/b8c5f36434d5d3fbfa4a](https://qiita.com/chibi929/items/b8c5f36434d5d3fbfa4a)

#### Installation of this sample
  * `$ git clone <URL>`
  * `$ make`
    * `$ ./example/dummy.cc` is compiled.
    * `$ ./bin/dummy` will be created.
  * `$ make TARGET=root_test`
    * `$ ./example/root_test.cc` is compiled.
    * `$ ./bin/root_test` will be created.
  
#### Using MyClass in CINT
  * `[maketest]$ root`
  * `root [0] #include "include/MyClass.h"`
  * `root [1] #include "src/MyClass.cc"`
  * `root [2] MyClass m`
  * `root [3] m.[MyClass func or valiable]` You can use "Tab" for autocompeletion. 
  -->
