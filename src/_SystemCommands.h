class _BreakHereFunction : public Function
{
  private:
    HSQUIRRELVM _vm;

  public:
    explicit _BreakHereFunction(HSQUIRRELVM vm)
        : _vm(vm)
    {
    }

    void operator()() override
    {
        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
        {
            std::cerr << "_BreakHereFunction: thread not suspended" << std::endl;
            sqstd_printcallstack(_vm);
        }
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakHereFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakWhileAnimatingFunction : public Function
{
  private:
    HSQUIRRELVM _vm;
    GGActor &_actor;

  public:
    explicit _BreakWhileAnimatingFunction(HSQUIRRELVM vm, GGActor &actor)
        : _vm(vm), _actor(actor)
    {
    }

    bool isElapsed() override
    {
        bool isPlaying = _actor.getCostume().getAnimation()->isPlaying();
        return !isPlaying;
    }

    void operator()() override
    {
        if (!isElapsed())
            return;

        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakWhileAnimatingFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakWhileWalkingFunction : public Function
{
  private:
    HSQUIRRELVM _vm;
    GGActor &_actor;

  public:
    explicit _BreakWhileWalkingFunction(HSQUIRRELVM vm, GGActor &actor)
        : _vm(vm), _actor(actor)
    {
    }

    bool isElapsed() override
    {
        auto name = _actor.getCostume().getAnimationName();
        bool isElapsed = name != "walk";
        return isElapsed;
    }

    void operator()() override
    {
        if (!isElapsed())
            return;

        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakWhileWalkingFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakWhileTalkingFunction : public Function
{
  private:
    HSQUIRRELVM _vm;
    GGActor &_actor;

  public:
    explicit _BreakWhileTalkingFunction(HSQUIRRELVM vm, GGActor &actor)
        : _vm(vm), _actor(actor)
    {
    }

    bool isElapsed() override
    {
        return !_actor.isTalking();
    }

    void operator()() override
    {
        if (!isElapsed())
            return;

        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakWhileTalkingFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakTimeFunction : public TimeFunction
{
  private:
    HSQUIRRELVM _vm;

  public:
    _BreakTimeFunction(HSQUIRRELVM vm, const sf::Time &time)
        : TimeFunction(time), _vm(vm)
    {
    }

    void operator()() override
    {
        if (isElapsed())
        {
            if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            {
                std::cerr << "_BreakTimeFunction: thread not suspended" << std::endl;
                sqstd_printcallstack(_vm);
            }
            if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
            {
                std::cerr << "_BreakTimeFunction: failed to wakeup: " << _vm << std::endl;
                sqstd_printcallstack(_vm);
            }
        }
    }
};

static SQInteger _breakwhileanimating(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::make_unique<_BreakWhileAnimatingFunction>(v, *pActor));
    return result;
}

static SQInteger _breakwhilewalking(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::make_unique<_BreakWhileWalkingFunction>(v, *pActor));
    return result;
}

static SQInteger _breakwhiletalking(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::make_unique<_BreakWhileTalkingFunction>(v, *pActor));
    return result;
}

static SQInteger _stopThread(HSQUIRRELVM v)
{
    HSQOBJECT thread_obj;
    sq_resetobject(&thread_obj);
    if (SQ_FAILED(sq_getstackobj(v, 2, &thread_obj)))
    {
        return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }
    sq_release(thread_obj._unVal.pThread, &thread_obj);
    sq_pop(thread_obj._unVal.pThread, 1);
    sq_suspendvm(thread_obj._unVal.pThread);
    // sq_close(thread_obj._unVal.pThread);
    return 0;
}

static SQInteger _startThread(HSQUIRRELVM v)
{
    SQInteger size = sq_gettop(v);

    // create thread and store it on the stack
    auto thread = sq_newthread(v, 1024);
    HSQOBJECT thread_obj;
    sq_resetobject(&thread_obj);
    if (SQ_FAILED(sq_getstackobj(v, -1, &thread_obj)))
    {
        return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }

    std::vector<HSQOBJECT> args;
    for (auto i = 0; i < size - 2; i++)
    {
        HSQOBJECT arg;
        sq_resetobject(&arg);
        if (SQ_FAILED(sq_getstackobj(v, 3 + i, &arg)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine args from stack"));
        }
        args.push_back(arg);
    }

    // get the closure
    HSQOBJECT closureObj;
    sq_resetobject(&closureObj);
    if (SQ_FAILED(sq_getstackobj(v, 2, &closureObj)))
    {
        return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }

    // call the closure in the thread
    sq_pushobject(thread, closureObj);
    sq_pushroottable(thread);
    for (auto arg : args)
    {
        sq_pushobject(thread, arg);
    }
    if (SQ_FAILED(sq_call(thread, 1 + args.size(), SQFalse, SQTrue)))
    {
        sq_throwerror(v, _SC("call failed"));
        sq_pop(thread, 1); // pop the compiled closure
        return SQ_ERROR;
    }

    sq_addref(v, &thread_obj);
    sq_pushobject(v, thread_obj);

    return 1;
}

static SQInteger _breakHere(HSQUIRRELVM v)
{
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::make_unique<_BreakHereFunction>(v));
    return result;
}

static SQInteger _breakTime(HSQUIRRELVM v)
{
    SQFloat time = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &time)))
    {
        return sq_throwerror(v, _SC("failed to get time"));
    }
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::make_unique<_BreakTimeFunction>(v, sf::seconds(time)));
    return result;
}
