# lock-free stack

Задание №3: Необходимо реализовать lock-free стэк фиксированного размера (без приоритетов).
Реализовать программу, демонстрирующую корректность работы стэка.

Интерфейс должен быть следующим:

```cpp
template <class T>
class LockFreeStack {
public:
    // конструктор стэка с заданной емкостью
    LockFreeStack( int capacity );
    
    // добавить элемент в стэк
    void Push(T value);
    
    // взять элемент из стэка
    T Pop();
    
    // проверяет, пустой ли стэк
    bool IsEmpty();
    
    // очищает стэк
    void Clear();
}
```
