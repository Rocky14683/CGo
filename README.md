# CGo - Write golang style code in c++

---
This repository is a not only for the purpose of writing golang style code in c++, 
but also a proof of concept for the idea that modern c++ can do anything.
---
## Channels
```cpp
    auto chanInt = Channel<int>::make_chan();
    chanInt <- -42;
    chanInt <- 43;

    int c;
    c <- chanInt;
    std::print("{}\n", c);
    c <- chanInt;
    std::print("{}\n", c);
```

```go
    var chanInt = make(chan int)
    chanInt <- -42
    chanInt <- 43
    
    c := <-chanInt
    fmt.Println(c)
    c = <-chanInt
    fmt.Println(c)
```
Output:
```
-42
43
```

## defer
```cpp
    defer(std::print("deferred\n"));
    defer([]() {
        std::print("deferred lambda\n");
    })
    std::print("hello\n");
```

```go
    defer fmt.Println("deferred")
    defer func() {
        fmt.Println("deferred lambda")
    }()
    fmt.Println("hello")
```
Output:
```
hello
deferred lambda
deferred
```
## TODO:
- good goroutine
- error handling
- tuple
- golang style build system

