# 简介

这是中国科学技术大学2023年秋编译原理课程（郑启龙 班）的一个小组实验。

> 小组成员：
>
> * 邓博文
> * 晏铭
> * 蒋文浩

关于实验内容和要求，请看 [2023-compiler-lab.pdf](Docs/2023-compiler-lab.pdf) 以及 [编译原理和技术实践2017.pdf](Docs/编译原理和技术实践2017.pdf) ，关于我们的具体实现，请看Docs/report.ppt (TODO)。



### 我们所做的内容

我们使用C++完全重写了pl0的编译器和解释器（分别位于`Compiler/`目录下和`Interpreter/`目录下），使得代码结构更加直观易懂，可供以后的学弟学妹参考以及进一步改进。

按[2023-compiler-lab.pdf](Docs/2023-compiler-lab.pdf) 文档要求，添加了对数组、指针以及作用域算符的支持。这里的实现支持任意维度的数组；赋值语句 ”:=“ 的左侧以及取地址符 ”&“ 后支持任意的表达式（只要结果为左值），例如可以编译像下面这样的pl0源码：

```
var *p;
var a[10];
begin
p := &a[0];
p := p+1;
*(p+1) := 7;
print(a[2]);	//输出7
end.
```



# 使用Visual Studio编译

这些代码本身是一个Visual Studio解决方案，用Visual Studio可以很容易地编译。

只需双击根目录下的`Compiler-Lab.sln`文件即可打开解决方案，在Visual Studio的菜单栏中点击`生成->生成解决方案`即可生成。生成的可执行程序`Compiler.exe`和`Interpreter.exe`位于`x64/Debug`目录下。



# 使用VSCode查看源码并使用g++编译

当然，你也可以不使用Visual Studio而是用VSCode与g++。

当用VSCode打开这些源码时，也许会出现乱码，这时你需要在VSCode编辑器中手动选择编码方式。

使用以下命令编译（假设位于该仓库的根目录下）：

**对于Windows系统**

```shell
g++ Compiler/*.cpp -IShared/ -o Compiler.exe -std=c++20
g++ Interpreter/*.cpp -IShared/ -o Interpreter.exe -std=c++20
```

**对于Linux系统**

```shell
g++ Compiler/*.cpp -IShared/ -o Compiler -std=c++20
g++ Interpreter/*.cpp -IShared/ -o Interpreter -std=c++20
```

注意使用的g++版本需要支持C++ 20。



# 如何使用pl0编译器和解释器

假设Compiler.exe、Interpreter.exe和源码文件（假设叫做example.txt）位于当前目录下。

```shell
./Compiler example.txt test # 编译得到二进制文件
./Interpreter test			# 使用pl0解释器运行
```

