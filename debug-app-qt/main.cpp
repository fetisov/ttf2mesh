#include "mainwindow.h"
#include <QApplication>

class Event
{
public:
    template<typename C> void subscr(C *obj, void (C::*method)(void *));
    template<typename C> void unsubs(C *obj, void (C::*method)(void *));
    inline void subscr(void (*proc)(void *));
    inline void unsubs(void (*proc)(void *));
    inline void call(void *sender = NULL);
    Event() {}
    ~Event();

private:
    Event(const Event &) {}
    Event &operator =(const Event &) { return *this; }
    class AbstractMethod
    {
    public:
        bool isStatic;
        virtual void call(void *sender = NULL) = 0;
        virtual ~AbstractMethod() {}
        virtual bool compare(AbstractMethod *other) const = 0;
    };
    template <typename C> class Method: public AbstractMethod
    {
    public:
        Method(C *obj, void (C::*method)(void *)) : obj(obj), method(method) { isStatic = false; }
        virtual void call(void *sender = NULL) { (obj->*method)(sender); }
        virtual bool compare(AbstractMethod *other) const
        {
            if (other->isStatic) return false;
            Method *o = static_cast<Method *>(other);
            return o->obj == obj && o->method == method;
        }
    private:
        C *obj;
        void (C::*method)(void *);
    };
    class StaticMethod: public AbstractMethod
    {
    public:
        StaticMethod(void (*proc)(void *)) : proc(proc) { isStatic = true; }
        virtual void call(void *sender = NULL) { proc(sender); }
        virtual bool compare(AbstractMethod *other) const
        {
            if (!other->isStatic) return false;
            StaticMethod *o = static_cast<StaticMethod *>(other);
            return o->proc == proc;
        }
    private:
        void (*proc)(void *);
    };
    std::vector<AbstractMethod *> methods;
};

Event::~Event()
{
    for (int i = 0; i < (int)methods.size(); i++)
        delete methods[i];
}

inline void Event::call(void *sender)
{
    for (int i = 0; i < (int)methods.size(); i++)
        methods[i]->call(sender);
}

template<typename C> void Event::subscr(C *obj, void (C::*method)(void *))
{
    methods.push_back(new Method<C>(obj, method));
}

inline void Event::subscr(void (*method)(void *))
{
    methods.push_back(new StaticMethod(method));
}

template<typename C> void Event::unsubs(C *obj, void (C::*method)(void *))
{
    Method<C> m(obj, method);
    int j = 0;
    for (int i = 0; i < (int)methods.size(); i++)
    {
        if (m.compare(methods[i]))
            delete methods[i]; else
            methods[j++] = methods[i];
    }
    methods.resize(j);
}

inline void Event::unsubs(void (*method)(void *))
{
    StaticMethod m(method);
    int j = 0;
    for (int i = 0; i < (int)methods.size(); i++)
    {
        if (m.compare(methods[i]))
            delete methods[i]; else
            methods[j++] = methods[i];
    }
    methods.resize(j);
}

namespace pts {

class Task
{
public:
    struct Info
    {
        static Info *table();
        static int tableSize();
    };

};

class Driver
{
public:
    Event onRunStateChanged;
    Event onTaskStarted;
    Event onTaskFinished;
    Event onMeasurement;
    void run(const std::vector<Task> &tasks);
    static inline Driver &instance()
    {
        static Driver res;
        return res;
    }

private:
    Driver() {}
    Driver(const Driver &) {}
    Driver &operator =(const Driver &) { return *this; }
};

//class TcpServerUser
//{
//};

//class TcpClientUser
//{
//};

//class Client
//{
//public:
//    Event onDisconnect;
//    Event onStateRequest;
//    Event onRunRequest;
//    Event onPauseRequest;
//    Event onAbortRequest;
//    Event onMessagesRequest;
//    void send();
//};

extern Driver &driver;

Driver &driver = Driver::instance();

}

namespace coi {

class Driver
{
public:
    Event onConnection;
};

}

void aaaa(void *)
{
}

int main(int argc, char *argv[])
{
    pts::driver.onRunStateChanged.subscr(&aaaa);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
