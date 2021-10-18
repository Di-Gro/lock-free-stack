#include <iostream>
#include <vector>
#include <future>

#include "LockFreeStack.h"

using namespace std;


LockFreeStack<int> lfstack{ 100 };

class Action {
public:
    enum class Type { Push, Pop, Clear };
    Type type;
    int value;
    int iteration;
    int thread;

    Action(int thread, int iteration, int value, Type type)
        : type(type), value(value), iteration(iteration), thread(thread) {}
};

vector<Action> threadFunc(int id, int minValue, int delay, int clearIteration) {
    vector<Action> res;

    for (int i = 0; i < 10; i++) {
        int c = 0;
        if (i == clearIteration) {
            lfstack.Clear(c);
            res.emplace_back(id, c, 0, Action::Type::Clear);
        }
        else {
            int v = minValue + i;
            lfstack.Push(v, c);
            res.emplace_back(id, c, v, Action::Type::Push);
        }
        this_thread::sleep_for(chrono::milliseconds(delay));
    }

    for (int i = 0; i < 10; i++) {
        if (!lfstack.IsEmpty()) {
            int c = 0;
            int v = lfstack.Pop(c);
            res.emplace_back(id, c, v, Action::Type::Pop);
        }
        this_thread::sleep_for(chrono::milliseconds(delay));
    }

    return res;
}

int main() {

    future<vector<Action>> future1 = async(threadFunc, 1, 10, 250, -1);
    future<vector<Action>> future2 = async(threadFunc, 2, 20, 100, 7);

    vector<Action> res1 = future1.get();
    vector<Action> res2 = future2.get();

    vector<Action> hist;
    hist.insert(hist.end(), res1.begin(), res1.end());
    hist.insert(hist.end(), res2.begin(), res2.end());

    sort(hist.begin(), hist.end(), [](Action& a, Action& b) {
        return a.iteration < b.iteration;
    });

    int head = -1;
    int restored[100];

    cout << "[thread] action (value): stack" << "\n";

    for (int i = 0; i < hist.size(); i++) {
        if (hist[i].type == Action::Type::Push) {
            head++;
            restored[head] = hist[i].value;
            cout << "[" << hist[i].thread << "] push (" << hist[i].value << "): ";
        }
        if (hist[i].type == Action::Type::Pop) {
            head--;
            cout << "[" << hist[i].thread << "] pop  (" << hist[i].value << "): ";
        }
        if (hist[i].type == Action::Type::Clear) {
            head = -1;
            cout << "[" << hist[i].thread << "] clear: ";
        }

        for (int k = head; k >= 0; k--)
            cout << restored[k] << " ";

        cout << "\n";
    }

}
