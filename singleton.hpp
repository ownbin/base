/*
* singleton.hpp - implementation of Singleton Pattern, thread-safety
* 
* Copyright (c) 2015 Ownbin Ho
*
* Distributed under the BSD License.
* 
* by Ownbin Ho (ownbin at hotmail dot com)
*/

#ifndef BASE_UTILS_SINGLETON_HPP
#define BASE_UTILS_SINGLETON_HPP

#include <boost/noncopyable.hpp>
#if defined(_MSC_VER) && (_MSC_VER < 1800) // MS VC++ 12.0 (Visual C++ 2013)
#include <boost/thread.hpp>
#endif

namespace utils
{
    /*
    * 单例模板
    *
    * 需求
    *
    * 1. 延迟加载，懒汉模式
    *       懒汉模式，延迟加载，静态成员变量指针初始化为空指针，待第一次调用是创
    *       建并赋值
    *
    * 2. 单例对象构造过程中有关联访问时，保持单例对象初始化有序，避免关联单例对
    *       象出现未初始化
    *   （此问题出现在饿汉模式中，由于使用了静态成员变量，程序开始时即进行初始化，
    *    因初始化无序，可能出现要访问的单例对象未初始化）
    *
    * 3. 自行析构，释放资源
    *
    * 使用
    *
    *     构造和析构为 protected，可同时支持继承和组合使用，如果构造和析构为 
    *     private，则只支持组合使用
    *
    *     推荐组合使用，解耦了目标类和模板的实现，且不影响目标类的继承体系
    */

#if defined(_MSC_VER) && (_MSC_VER < 1800)

    /*
    * 懒汉模式
    *
    * 使用局部静态变量实现，需确保线程安全性，避免多线程时重复初始化或返回未初始
    * 化完成的对象
    *
    * C++11标准规定了局部静态变量初始化的线程安全性，之前的版本并无规定，则需自
    * 行实现保证线程安全性
    *
    * 说明：
    *   访问的时候使用了double-check，避免了每次获取验证锁的控制，提高访问效率
    *   借助局部静态变量的自行析构，实现单例对象的析构（调用 atexit() 也可），
    *   静态局部变量调用一次空方法，避免s_object_destroyer_被编译器优化而不会创建
    *
    * 此实现的优势有，延迟加载，节省资源，线程安全，资源自行释放，且不会出现饿汉
    * 模式中对象构造时关联访问未初始化的问题
    */

    template<typename T>
    class singleton : public boost::noncopyable
    {
    public:
        static T& instance()
        {
            if (object_ == NULL)
            {
                boost::mutex::scoped_lock lock(mutex_);
                if (object_ == NULL)
                {
                    object_ = new T;
                    destroy_object_.do_nothing(); 
                }
            }
            return *object_;
        }

    private:
        singleton() {}
        ~singleton() {}

        static void destroy()
        {
            delete object_;
            object_ = NULL;
        }

        static T* object_;
        static boost::mutex mutex_;

        struct object_destroyer
        {
            ~object_destroyer()
            {
                if (singleton<T>::object_ != NULL)
                {
                    singleton<T>::destroy();
                }
            }

            inline void do_nothing() const {};
        };
        static object_destroyer destroy_object_;
    };

    template<typename T> 
    T* singleton<T>::object_ = NULL;

    template<typename T> 
    boost::mutex singleton<T>::mutex_;

    template<typename T> 
    typename singleton<T>::object_destroyer singleton<T>::destroy_object_;

    /*
    * 实现方式2：饿汉模式
    *
    * 使用静态成员变量，程序开始时初始化即初始化单例对象，如果一个单例对象的构造
    * 函数中访问了另一个单例对象，此时另一个单例对象可能还未构造
    * 使用 object_creator 包装实例构造，使单例对象的初始化有序，避免此问题
    * 
    * 此方式为原boost库实现方式
    *
    * 优点：线程安全，资源自行释放
    * 缺点：当单例对象未使用时也会创建，浪费资源
    */
    /*
    template<typename T>
    class singleton : public boost::noncopyable
    {
    public:
        static T& instance()
        {
            static T object;
            create_object_.do_nothing();
            return object;
        }

    private:
        singleton() {}
        ~singleton() {}

        struct object_creator
        {
            object_creator() 
            { 
                singleton<T>::instance(); 
            }
            inline void do_nothing() const {}
        };
        static object_creator create_object_;
    };

    template<typename T>
    typename singleton<T>::object_creator singleton<T>::create_object_;
    */

#else // defined(__cplusplus) && (__cplusplus >= 201103L)

    template<typename T>
    class singleton : public boost::noncopyable
    {
    public:
        static T& instance()
        {
            static T object;
            return object;
        }

    private:
        singleton() {}
        ~singleton() {}
    };

#endif

    // use macro SINGLETON to call the instance always
#define SINGLETON(classname) utils::singleton<classname>
    // if the constructor of class is private, use the FRIEND_SINGLETON in the 
    // definition of the class
#define FRIEND_SINGLETON(classname) friend class utils::singleton<classname>;

} // namespace utils

#endif
