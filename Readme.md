# 基于 C 的自制shell 程序

解析用户输入的命令，将其转化成命令参数执行

- API：fork、execve、wait、signal、dup .etc

## 使用方法
```
# clone repository
git clone https://github.com/ruoshui255/shell.git

# build and run
make run
```

## 实现的功能

### 短路求值功能
![](./img/%E7%9F%AD%E8%B7%AF%E6%B1%82%E5%80%BC.gif)

### 内置命令

cd、jobs、bg、fg、exit等内置命令

![](./img/%E5%86%85%E7%BD%AE%E5%91%BD%E4%BB%A4.gif)


### 重定向功能
管道、输入重定向、输出重定向、后台执行等

![](./img/%E9%87%8D%E5%AE%9A%E5%90%91.gif)