#ifndef __JOB_HPP__
#define __JOB_HPP__

#include "ant.hpp"

class Job
{
protected:
    // Including walk, job, management
    virtual void work() = 0;
    // Eat the food which it get
    virtual void eat() = 0;
    virtual int get_food() = 0;
    // Set live status in each work being done
    virtual void alive_handler() = 0;
    virtual void clean() = 0;

public:
    virtual ~Job() {}
    Job() = default;
    Job(Job &&j) = default;
    // Each ant will be called by `do_job()`
    void do_job()
    {
        work();
        eat();
        alive_handler();
    }
    virtual void info() {}
};

#endif