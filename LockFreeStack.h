#pragma once
#include <atomic>

using namespace std;


template<typename T>
class LockFreeStack {
private:
    static const int USED_HEAD = 0;
    static const int FREE_HEAD = 1;

    class Node {
    public:
        T value;
        Node* next;
    };

    class Data {
    public:
        shared_ptr<Node[]> nodes;
        Node* heads[2] = { nullptr, nullptr };

        Data() {}
        Data(const Data& other) {
            nodes = shared_ptr<Node[]>(other.nodes);
            heads[0] = other.heads[0];
            heads[1] = other.heads[1];
        }
    };

    class DataHeader {
    public:
        int version = 0;
        Data* data = nullptr;

        Node*& head(int head) { return data->heads[head]; }
    };

    atomic<DataHeader> m_header;
    int m_capacity;

public:

    LockFreeStack(int capacity)  {
        m_capacity = capacity;

        DataHeader header;
        header.data = new Data();
        m_InitData(*header.data, m_capacity);

        m_header.store(header, memory_order_relaxed);
    }

    void Push(T value, int& version) {
        Node* node = m_Pop(FREE_HEAD, version);
        if (node == nullptr)
            return;

        node->value = move(value);
        m_Push(USED_HEAD, node, version);
    }

    void Push(T value) {
        int v;
        Push(value, v);
    }

    T Pop(int& version) {
        Node* node = m_Pop(USED_HEAD, version);
        if (node == nullptr)
            return false;

        T value = move(node->value);
        m_Push(FREE_HEAD, node, version);
        return value;
    }

    T Pop() {
        int v;
        return Pop(v);
    }

    bool IsEmpty() {
        DataHeader header = m_header.load(memory_order_relaxed);
        return header.head(USED_HEAD) == nullptr;
    }

    void Clear(int& version) {
        m_Clear(version);
    };

    void Clear() {
        int v;
        m_Clear(v);
    };

    ~LockFreeStack() {
       DataHeader header = m_header.load(memory_order_relaxed);
       delete header.data;
    }

private:

    void m_InitData(Data& data, int capacity) {
        data.nodes.reset(new Node[capacity]);
        data.nodes[capacity - 1].next = nullptr;
        for (int i = 0; i < capacity - 1; i++)
            data.nodes[i].next = &data.nodes[i + 1];

        data.heads[USED_HEAD] = nullptr;
        data.heads[FREE_HEAD] = &data.nodes[0];
    }

    bool m_CompareHeader(atomic<DataHeader>& a, DataHeader& b, DataHeader& value) {
        return a.compare_exchange_weak(b, value, memory_order_acq_rel, memory_order_acquire);
    }

    Node* m_Pop(int headId, int& version) {
        DataHeader last = m_header.load(memory_order_relaxed);
        DataHeader next;
        next.data = new Data(*last.data);
        do {
            if (last.head(headId) == nullptr) {
                delete next.data;
                return nullptr;
            }
            next.version = last.version + 1;
            next.head(headId) = last.head(headId)->next;
            version = next.version;
        } while (!m_CompareHeader(m_header, last, next));

        delete last.data;
        return last.head(headId);
    }

    Node* m_Pop(int headId) {
        int v;
        return m_Pop(headId, v);
    }

    void m_Push(int headId, Node* node, int& version) {
        DataHeader last = m_header.load(memory_order_relaxed);
        DataHeader next;
        next.data = new Data(*last.data);
        do {
            node->next = last.head(headId);
            next.version = last.version + 1;
            next.head(headId) = node;
            version = next.version;
        } while (!m_CompareHeader(m_header, last, next));

        delete last.data;
    }

    void m_Push(int headId, Node* node) {
        int v;
        m_Push(node, headId, v);
    }

    void m_Clear(int& version) {
        DataHeader last = m_header.load(memory_order_relaxed);
        DataHeader next;
        next.data = new Data();
        m_InitData(*next.data, m_capacity);
        do {
            next.version = last.version + 1;
            version = next.version;
        } while (!m_CompareHeader(m_header, last, next));

        delete last.data;
    }
};
